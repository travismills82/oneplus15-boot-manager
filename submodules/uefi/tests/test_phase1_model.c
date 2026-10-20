/* SPDX-License-Identifier: BSD-3-Clause */
#include <assert.h>
#include <string.h>
#include "../edk2/QcomModulePkg/Application/LinuxLoader/SuperFbPhase1Model.h"

static SFB_PHASE1_SLOT_FACTS Ready (void)
{
  SFB_PHASE1_SLOT_FACTS Facts = {TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, 3};
  return Facts;
}

int main (void)
{
  SFB_PHASE1_MODEL Model;
  SFB_PHASE1_SLOT_FACTS A = Ready ();
  SFB_PHASE1_SLOT_FACTS B = Ready ();

  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (strcmp (Model.Entry[0].Id, "android-a") == 0);
  assert (strcmp (Model.Entry[4].Id, "fastbootd") == 0);
  assert (Model.Entry[0].Availability == SfbPhase1Ready);
  assert (Model.Entry[0].Active && Model.Entry[0].Default);
  assert (Model.Entry[1].Availability == SfbPhase1InactiveSlot);
  assert (!Model.Entry[1].Default);
  assert (Model.Entry[2].Type == SfbPhase1Recovery);
  assert (Model.Entry[3].Type == SfbPhase1SuperFastboot);
  assert (Model.Entry[4].Type == SfbPhase1Fastbootd);

  SfbPhase1BuildModel (SFB_PHASE1_SLOT_B, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1InactiveSlot);
  assert (!Model.Entry[0].Default); /* default never implies a GPT transition */
  assert (Model.Entry[1].Availability == SfbPhase1Ready);

  SfbPhase1BuildModel (SFB_PHASE1_NO_SLOT, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1ActiveSlotUnknown);
  assert (Model.Entry[1].Availability == SfbPhase1ActiveSlotUnknown);

  A.LoaderPresent = FALSE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1LoaderMissing);
  A = Ready (); A.ImageValid = FALSE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1LoaderInvalid);
  A = Ready (); A.ProfileValid = FALSE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1ProfileInvalid);
  A = Ready (); A.TzMapValid = FALSE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1TzMapInvalid);
  A = Ready (); A.RetryKnown = FALSE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1RetryUnknown);
  A = Ready (); A.Unbootable = TRUE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1SlotUnbootable);
  A = Ready (); A.RetryCount = 0;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1RetriesExhausted);
  A.Successful = TRUE;
  SfbPhase1BuildModel (SFB_PHASE1_SLOT_A, SFB_PHASE1_SLOT_A, &A, &B, &Model);
  assert (Model.Entry[0].Availability == SfbPhase1Ready);

  return 0;
}
