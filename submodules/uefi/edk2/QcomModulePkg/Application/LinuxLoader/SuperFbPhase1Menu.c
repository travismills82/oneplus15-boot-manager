/*
 * OnePlus 15 Phase 1 A/B selector.
 *
 * This frontend reuses Canoe's native UEFI console, input, loaders and reboot
 * target library. It deliberately exposes no GPT writer and never treats an
 * inactive loader filename as evidence that the rest of the boot chain has
 * changed slots.
 *
 * Copyright (c) 2026, contributors to the canoe ABL tree.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "SuperFbPhase1Menu.h"
#include "SuperFbPhase1Model.h"
#include "SuperFbLaunchPolicy.h"
#include "SuperFbSlots.h"
#include "SuperFbLastBoot.h"

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/RebootTargetLib.h>
#include <Library/ShutdownServices.h>

#define SFB_PHASE1_ROW_CHARS 112

typedef struct {
  SFB_MENU_STATE       Source;
  SFB_PHASE1_MODEL     Model;
  SFB_MENU_TEMPLATE   *Template;
  SFB_BOOT_MODE        Mode;
  UINTN                SlotEntry[2];
  UINTN                DefaultIndex;
  BOOLEAN              EnterFastboot;
  CHAR16               RowText[SFB_PHASE1_ENTRY_COUNT][SFB_PHASE1_ROW_CHARS];
} SFB_PHASE1_CONTEXT;

STATIC CONST CHAR16 *mSfbPhase1Names[SFB_PHASE1_ENTRY_COUNT] = {
  L"Boot Android - Slot A",
  L"Boot Android - Slot B",
  L"Boot Recovery",
  L"Boot Super Fastboot",
  L"Boot Fastbootd"
};

STATIC CONST CHAR16 *mSfbPhase1Descriptions[SFB_PHASE1_ENTRY_COUNT] = {
  L"Launch the validated Slot A loader and its matching sidecars.",
  L"Launch the validated Slot B loader and its matching sidecars.",
  L"Set the standard one-time recovery request; do not change the default OS.",
  L"Enter CANOE-BDS resident Super Fastboot.",
  L"Set the standard one-time userspace fastboot request; preserve Mode 2 policy."
};

STATIC
CONST CHAR16 *
SfbPhase1Reason (IN SFB_PHASE1_AVAILABILITY Availability)
{
  switch (Availability) {
  case SfbPhase1Ready: return L"Available";
  case SfbPhase1InactiveSlot:
    return L"Unavailable: inactive-slot switching is not verified in Phase 1";
  case SfbPhase1ActiveSlotUnknown:
    return L"Unavailable: active slot could not be established from GPT";
  case SfbPhase1LoaderMissing:
    return L"Unavailable: prepared slot loader is missing";
  case SfbPhase1LoaderInvalid:
    return L"Unavailable: loader is not an AArch64 UEFI application";
  case SfbPhase1ProfileInvalid:
    return L"Unavailable: matching .gm2p sidecar is missing or invalid";
  case SfbPhase1TzMapInvalid:
    return L"Unavailable: matching .tzmap sidecar is missing or invalid";
  case SfbPhase1RetryUnknown:
    return L"Unavailable: slot retry metadata could not be read";
  case SfbPhase1SlotUnbootable:
    return L"Unavailable: GPT marks the active slot unbootable";
  case SfbPhase1RetriesExhausted:
    return L"Unavailable: active slot has no retry remaining";
  default: return L"Unavailable";
  }
}

STATIC
SFB_UINT8
SfbPhase1SlotNumber (IN SFB_SLOT Slot)
{
  return Slot == SfbSlotA ? SFB_PHASE1_SLOT_A :
         Slot == SfbSlotB ? SFB_PHASE1_SLOT_B : SFB_PHASE1_NO_SLOT;
}

STATIC
BOOLEAN
SfbPhase1PathIsSlot (IN CONST CHAR16 *Path, IN SFB_UINT8 Slot)
{
  return (BOOLEAN)(
    Path != NULL &&
    StrCmp (Path, Slot == SFB_PHASE1_SLOT_A
                    ? SFB_MANAGED_SLOT_A_NAME : SFB_MANAGED_SLOT_B_NAME) == 0);
}

STATIC
VOID
SfbPhase1InspectLoader (
  IN  CONST SFB_BOOT_ENTRY   *Entry,
  IN  SFB_SLOT_RETRY_STATE    Retry,
  OUT SFB_PHASE1_SLOT_FACTS  *Facts
  )
{
  SFB_MODE2_PROFILE Profile;
  SFB_TZ_MAP        TzMap;
  EFI_FILE_PROTOCOL *Root = NULL;

  ZeroMem (Facts, sizeof (*Facts));
  Facts->RetryKnown = Retry.Available;
  Facts->Successful = Retry.Successful;
  Facts->Unbootable = Retry.Unbootable;
  Facts->RetryCount = Retry.RetryCount;
  if (Entry == NULL) { return; }
  Facts->LoaderPresent = TRUE;
  if (EFI_ERROR (SfbOpenVolumeRoot (Entry->Volume, &Root)) || Root == NULL) {
    return;
  }
  Facts->ImageValid = SfbIsArm64EfiApplicationFile (Root, Entry->Path);
  Root->Close (Root);
  if (!Facts->ImageValid) { return; }
  Facts->ProfileValid = (SFB_BOOLEAN)!EFI_ERROR (
                                      SfbLoadMode2Profile (Entry, &Profile));
  Facts->TzMapValid = (SFB_BOOLEAN)!EFI_ERROR (SfbLoadTzMap (Entry, &TzMap));
}

STATIC
VOID
SfbPhase1BuildContext (
  OUT SFB_PHASE1_CONTEXT *State,
  IN  SFB_BOOT_MODE       Mode,
  IN  BOOLEAN             FirstRun
  )
{
  SFB_PHASE1_SLOT_FACTS FactsA;
  SFB_PHASE1_SLOT_FACTS FactsB;
  SFB_SLOT_RETRIES      Retries;
  SFB_UINT8             Active;
  SFB_UINT8             DefaultSlot = SFB_PHASE1_NO_SLOT;
  UINTN                 Index;

  ZeroMem (State, sizeof (*State));
  State->Mode = Mode;
  State->SlotEntry[0] = State->SlotEntry[1] = SFB_NO_INDEX;
  State->DefaultIndex = SFB_NO_INDEX;
  SfbBuildMenu (&State->Source, Mode, FirstRun);
  for (Index = 0; Index < State->Source.Count; Index++) {
    SFB_BOOT_ENTRY *Entry = &State->Source.Entry[Index];
    if (!SfbIsManagedAblEntry (Entry) || Entry->IsUsb) { continue; }
    if (State->SlotEntry[0] == SFB_NO_INDEX &&
        SfbPhase1PathIsSlot (Entry->Path, SFB_PHASE1_SLOT_A)) {
      State->SlotEntry[0] = Index;
    } else if (State->SlotEntry[1] == SFB_NO_INDEX &&
               SfbPhase1PathIsSlot (Entry->Path, SFB_PHASE1_SLOT_B)) {
      State->SlotEntry[1] = Index;
    }
  }
  if (State->Source.DefaultFromConfig &&
      State->Source.DefaultIndex < State->Source.Count) {
    CONST CHAR16 *Path = State->Source.Entry[State->Source.DefaultIndex].Path;
    if (SfbPhase1PathIsSlot (Path, SFB_PHASE1_SLOT_A)) {
      DefaultSlot = SFB_PHASE1_SLOT_A;
    } else if (SfbPhase1PathIsSlot (Path, SFB_PHASE1_SLOT_B)) {
      DefaultSlot = SFB_PHASE1_SLOT_B;
    }
  }

  Retries = SfbSlotRetries ();
  SfbPhase1InspectLoader (
    State->SlotEntry[0] == SFB_NO_INDEX ? NULL :
      &State->Source.Entry[State->SlotEntry[0]], Retries.A, &FactsA);
  SfbPhase1InspectLoader (
    State->SlotEntry[1] == SFB_NO_INDEX ? NULL :
      &State->Source.Entry[State->SlotEntry[1]], Retries.B, &FactsB);
  Active = SfbPhase1SlotNumber (SfbActiveSlot ());
  SfbPhase1BuildModel (Active, DefaultSlot, &FactsA, &FactsB, &State->Model);

  for (Index = 0; Index < SFB_PHASE1_ENTRY_COUNT; Index++) {
    CONST SFB_PHASE1_ENTRY *Entry = &State->Model.Entry[Index];
    UnicodeSPrint (
      State->RowText[Index], sizeof (State->RowText[Index]), L"%s%s%s%s",
      mSfbPhase1Names[Index],
      Entry->Active ? L"  [ACTIVE]" : L"",
      Entry->Default ? L"  [DEFAULT]" : L"",
      Entry->Availability == SfbPhase1Ready ? L"" : L"  [UNAVAILABLE]");
    if (Entry->Default) { State->DefaultIndex = Index; }
  }
  if (State->DefaultIndex == SFB_NO_INDEX) {
    if (State->Model.Entry[0].Availability == SfbPhase1Ready) {
      State->DefaultIndex = 0;
    } else if (State->Model.Entry[1].Availability == SfbPhase1Ready) {
      State->DefaultIndex = 1;
    } else {
      State->DefaultIndex = 3; /* Keep resident Super Fastboot selected. */
    }
  }
}

BOOLEAN
SfbPhase1BootOnceSelectorReady (
  IN CONST CHAR8  *Selector,
  IN SFB_BOOT_MODE Mode
  )
{
  SFB_PHASE1_CONTEXT State;
  BOOLEAN            Ready = FALSE;
  UINTN              Row;

  if (Selector == NULL || Selector[0] == '\0') { return FALSE; }
  SfbPhase1BuildContext (&State, Mode, FALSE);
  for (Row = 0; Row < 2; Row++) {
    UINTN SourceIndex = State.SlotEntry[Row];

    if (SourceIndex != SFB_NO_INDEX &&
        State.Model.Entry[Row].Availability == SfbPhase1Ready &&
        AsciiStrCmp (State.Source.Entry[SourceIndex].DefaultTarget,
                     Selector) == 0) {
      Ready = TRUE;
      break;
    }
  }
  SfbFreeMenu (&State.Source);
  return Ready;
}

STATIC
VOID
SfbPhase1DrawHeader (IN VOID *Context)
{
  SFB_PHASE1_CONTEXT *State = Context;
  UINTN               Row = State->Template->Cursor;
  CHAR16              SlotLine[64];
  CONST CHAR16       *Active = State->Model.ActiveSlot == SFB_PHASE1_SLOT_A
                                 ? L"A" :
                               State->Model.ActiveSlot == SFB_PHASE1_SLOT_B
                                 ? L"B" : L"unknown";
  UnicodeSPrint (SlotLine, sizeof (SlotLine), L"Current active slot: %s", Active);
  SfbDrawInfoLine (SlotLine);
  SfbDrawInfoLine (mSfbPhase1Descriptions[Row]);
  SfbDrawInfoLine (SfbPhase1Reason (State->Model.Entry[Row].Availability));
  SfbDrawInfoLine (L"--------");
}

STATIC
VOID
SfbPhase1DrawRow (IN VOID *Context, IN UINTN Row, IN BOOLEAN Selected)
{
  SFB_PHASE1_CONTEXT *State = Context;
  SfbDrawRow (Selected,
              State->Model.Entry[Row].Default ? L"*" : L" ",
              State->RowText[Row]);
}

STATIC
SFB_MENU_ACTION
SfbPhase1Handle (IN VOID *Context, IN UINTN Row, IN SFB_KEY Key)
{
  SFB_PHASE1_CONTEXT *State = Context;
  SFB_PHASE1_ENTRY   *Entry;
  EFI_STATUS          Status;
  UINT8               Reason;

  if (Row >= SFB_PHASE1_ENTRY_COUNT ||
      (Key != SfbKeySelect && Key != SfbKeyTimeout)) {
    return SfbMenuActionContinue;
  }
  Entry = &State->Model.Entry[Row];
  if (Entry->Availability != SfbPhase1Ready) {
    SfbReportStatus (SfbPhase1Reason (Entry->Availability), EFI_UNSUPPORTED);
    return SfbMenuActionContinue;
  }
  if (Entry->Type == SfbPhase1Android) {
    UINTN SourceIndex = State->SlotEntry[Entry->Slot - 1];
    if (SourceIndex == SFB_NO_INDEX) { return SfbMenuActionContinue; }
    SfbSetLaunchLockPolicy (State->Source.ConfigValid
                              ? State->Source.LockPolicy
                              : SfbConfigLockAsNeeded);
    Status = SfbLaunchEntryWithRetryPolicy (
               &State->Source.Entry[SourceIndex], TRUE, State->Mode, FALSE);
    SfbReportStatus (L"Android loader returned", Status);
    return SfbMenuActionContinue;
  }
  if (Entry->Type == SfbPhase1SuperFastboot) {
    State->EnterFastboot = TRUE;
    return SfbMenuActionExit;
  }
  Status = RebootTargetPrepare (
             Entry->Type == SfbPhase1Recovery
               ? RebootTargetRecovery : RebootTargetFastbootd,
             &Reason);
  if (EFI_ERROR (Status)) {
    SfbReportStatus (L"Could not prepare one-time boot request", Status);
    return SfbMenuActionContinue;
  }
  SfbBeginScreen (Entry->Type == SfbPhase1Recovery
                    ? L"Restarting into Recovery..."
                    : L"Restarting into Fastbootd...", NULL, NULL);
  RebootDevice (Reason);
  SfbReportStatus (L"Restart returned", EFI_DEVICE_ERROR);
  return SfbMenuActionContinue;
}

BOOLEAN
SfbRunPhase1BootMenu (
  IN SFB_BOOT_MODE InitialMode,
  IN BOOLEAN       AllowCountdown,
  IN BOOLEAN       FirstRun
  )
{
  SFB_PHASE1_CONTEXT State;
  SFB_MENU_TEMPLATE  Template;

  (VOID)SfbLastBootClear ();
  if (InitialMode > SfbBootModeKmProfile) {
    InitialMode = SfbBootModeAblFakeLocked;
  }
  ZeroMem (&Template, sizeof (Template));
  SfbPhase1BuildContext (&State, InitialMode, FirstRun);
  State.Template = &Template;
  Template.Title = L"ONEPLUS 15 BOOT MANAGER";
  Template.Subtitle = L"EXPERIMENTAL - NOT VALIDATED ON HARDWARE";
  Template.Footer = L"Volume Up/Down: move   Power: select   * configured default";
  Template.RowCount = SFB_PHASE1_ENTRY_COUNT;
  Template.Cursor = State.DefaultIndex;
  Template.Navigate = TRUE;
  Template.Context = &State;
  Template.Handler = SfbPhase1Handle;
  Template.DrawHeader = SfbPhase1DrawHeader;
  Template.DrawRow = SfbPhase1DrawRow;
  Template.ExtraRows = 5;
  Template.ShowCountdown = (BOOLEAN)(
    AllowCountdown && State.DefaultIndex < SFB_PHASE1_ENTRY_COUNT &&
    State.Model.Entry[State.DefaultIndex].Default);
  Template.TimeoutMs = Template.ShowCountdown
                         ? State.Source.MenuTimeoutSeconds * 1000u : 0;
  /* The scaffold accepts a custom drawer without a static Rows array;
   * RowText is owned by this invocation. */
  (VOID)SfbRunMenu (&Template);
  SfbFreeMenu (&State.Source);
  return State.EnterFastboot;
}

SFB_DEFAULT_RESULT
SfbLaunchPhase1Default (IN SFB_BOOT_MODE Mode)
{
  SFB_PHASE1_CONTEXT State;
  SFB_DEFAULT_RESULT Result = SfbDefaultNotFound;
  SFB_PHASE1_ENTRY  *Entry;
  UINTN              SourceIndex;

  SfbPhase1BuildContext (&State, Mode, FALSE);
  if (State.DefaultIndex >= SFB_PHASE1_ENTRY_COUNT) { goto Exit; }
  Entry = &State.Model.Entry[State.DefaultIndex];
  if (!Entry->Default || Entry->Type != SfbPhase1Android ||
      Entry->Availability != SfbPhase1Ready) {
    goto Exit;
  }
  SourceIndex = State.SlotEntry[Entry->Slot - 1];
  if (SourceIndex == SFB_NO_INDEX) { goto Exit; }
  SfbSetLaunchLockPolicy (State.Source.ConfigValid
                            ? State.Source.LockPolicy : SfbConfigLockAsNeeded);
  (VOID)SfbLaunchEntryWithRetryPolicy (
          &State.Source.Entry[SourceIndex], FALSE, Mode, FALSE);
  Result = SfbDefaultReturned;
Exit:
  SfbFreeMenu (&State.Source);
  return Result;
}
