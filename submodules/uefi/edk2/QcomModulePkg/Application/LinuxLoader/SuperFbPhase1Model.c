/* SPDX-License-Identifier: BSD-3-Clause */
#include "SuperFbPhase1Model.h"

static SFB_PHASE1_AVAILABILITY
SfbPhase1AssessSlot (
  SFB_UINT8                    Slot,
  SFB_UINT8                    ActiveSlot,
  const SFB_PHASE1_SLOT_FACTS *Facts
  )
{
  if (ActiveSlot != SFB_PHASE1_SLOT_A && ActiveSlot != SFB_PHASE1_SLOT_B) {
    return SfbPhase1ActiveSlotUnknown;
  }
  /* Phase 1 has no verified mechanism for coherently changing the Qualcomm
   * firmware and Android slot set. Keep the inactive row visible but inert. */
  if (Slot != ActiveSlot) { return SfbPhase1InactiveSlot; }
  if (Facts == NULL || !Facts->LoaderPresent) { return SfbPhase1LoaderMissing; }
  if (!Facts->ImageValid) { return SfbPhase1LoaderInvalid; }
  if (!Facts->ProfileValid) { return SfbPhase1ProfileInvalid; }
  if (!Facts->TzMapValid) { return SfbPhase1TzMapInvalid; }
  if (!Facts->RetryKnown) { return SfbPhase1RetryUnknown; }
  if (Facts->Unbootable) { return SfbPhase1SlotUnbootable; }
  if (!Facts->Successful && Facts->RetryCount == 0) {
    return SfbPhase1RetriesExhausted;
  }
  return SfbPhase1Ready;
}

void
SfbPhase1BuildModel (
  SFB_UINT8                   ActiveSlot,
  SFB_UINT8                   DefaultSlot,
  const SFB_PHASE1_SLOT_FACTS *SlotA,
  const SFB_PHASE1_SLOT_FACTS *SlotB,
  SFB_PHASE1_MODEL            *Model
  )
{
  static const SFB_PHASE1_ENTRY Definitions[SFB_PHASE1_ENTRY_COUNT] = {
    {"android-a", "Boot Android - Slot A", "Launch the validated Slot A loader.",
     "boot_a.efi + .gm2p + .tzmap", SfbPhase1Android, SFB_PHASE1_SLOT_A,
     SfbPhase1Ready, FALSE, FALSE},
    {"android-b", "Boot Android - Slot B", "Launch the validated Slot B loader.",
     "boot_b.efi + .gm2p + .tzmap", SfbPhase1Android, SFB_PHASE1_SLOT_B,
     SfbPhase1Ready, FALSE, FALSE},
    {"recovery", "Boot Recovery", "Write a one-time recovery BCB request and restart.",
     "misc BCB boot-recovery", SfbPhase1Recovery, SFB_PHASE1_NO_SLOT,
     SfbPhase1Ready, FALSE, FALSE},
    {"super-fastboot", "Boot Super Fastboot", "Enter CANOE-BDS resident fastboot.",
     "CANOE FastbootLib", SfbPhase1SuperFastboot, SFB_PHASE1_NO_SLOT,
     SfbPhase1Ready, FALSE, FALSE},
    {"fastbootd", "Boot Fastbootd", "Write a one-time userspace fastboot BCB request and restart.",
     "misc BCB boot-fastboot", SfbPhase1Fastbootd, SFB_PHASE1_NO_SLOT,
     SfbPhase1Ready, FALSE, FALSE}
  };
  SFB_UINTN Index;

  if (Model == NULL) { return; }
  for (Index = 0; Index < SFB_PHASE1_ENTRY_COUNT; Index++) {
    Model->Entry[Index] = Definitions[Index];
  }
  Model->ActiveSlot = ActiveSlot;
  Model->DefaultSlot = DefaultSlot;
  Model->Entry[0].Availability = SfbPhase1AssessSlot (
                                  SFB_PHASE1_SLOT_A, ActiveSlot, SlotA);
  Model->Entry[1].Availability = SfbPhase1AssessSlot (
                                  SFB_PHASE1_SLOT_B, ActiveSlot, SlotB);
  Model->Entry[0].Active = (SFB_BOOLEAN)(ActiveSlot == SFB_PHASE1_SLOT_A);
  Model->Entry[1].Active = (SFB_BOOLEAN)(ActiveSlot == SFB_PHASE1_SLOT_B);
  Model->Entry[0].Default = (SFB_BOOLEAN)(
                              DefaultSlot == SFB_PHASE1_SLOT_A &&
                              Model->Entry[0].Availability == SfbPhase1Ready);
  Model->Entry[1].Default = (SFB_BOOLEAN)(
                              DefaultSlot == SFB_PHASE1_SLOT_B &&
                              Model->Entry[1].Availability == SfbPhase1Ready);
}
