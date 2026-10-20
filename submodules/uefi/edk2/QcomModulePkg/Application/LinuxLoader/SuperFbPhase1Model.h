/*
 * Pure Phase 1 A/B menu policy. This file intentionally has no EDK2
 * dependency so the fail-closed slot rules can be exercised on the host.
 *
 * Copyright (c) 2026, contributors to the canoe ABL tree.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SUPER_FB_PHASE1_MODEL_H__
#define __SUPER_FB_PHASE1_MODEL_H__

#include "Hook/SuperFbProfile.h"

#define SFB_PHASE1_ENTRY_COUNT 5u
#define SFB_PHASE1_NO_SLOT     0u
#define SFB_PHASE1_SLOT_A      1u
#define SFB_PHASE1_SLOT_B      2u

typedef enum {
  SfbPhase1Android = 0,
  SfbPhase1Recovery,
  SfbPhase1SuperFastboot,
  SfbPhase1Fastbootd
} SFB_PHASE1_ENTRY_TYPE;

typedef enum {
  SfbPhase1Ready = 0,
  SfbPhase1InactiveSlot,
  SfbPhase1ActiveSlotUnknown,
  SfbPhase1LoaderMissing,
  SfbPhase1LoaderInvalid,
  SfbPhase1ProfileInvalid,
  SfbPhase1TzMapInvalid,
  SfbPhase1RetryUnknown,
  SfbPhase1SlotUnbootable,
  SfbPhase1RetriesExhausted
} SFB_PHASE1_AVAILABILITY;

typedef struct {
  SFB_BOOLEAN LoaderPresent;
  SFB_BOOLEAN ImageValid;
  SFB_BOOLEAN ProfileValid;
  SFB_BOOLEAN TzMapValid;
  SFB_BOOLEAN RetryKnown;
  SFB_BOOLEAN Successful;
  SFB_BOOLEAN Unbootable;
  SFB_UINT8   RetryCount;
} SFB_PHASE1_SLOT_FACTS;

typedef struct {
  const char                  *Id;
  const char                  *DisplayName;
  const char                  *Description;
  const char                  *RequiredData;
  SFB_PHASE1_ENTRY_TYPE        Type;
  SFB_UINT8                    Slot;
  SFB_PHASE1_AVAILABILITY      Availability;
  SFB_BOOLEAN                 Active;
  SFB_BOOLEAN                 Default;
} SFB_PHASE1_ENTRY;

typedef struct {
  SFB_PHASE1_ENTRY Entry[SFB_PHASE1_ENTRY_COUNT];
  SFB_UINT8        ActiveSlot;
  SFB_UINT8        DefaultSlot;
} SFB_PHASE1_MODEL;

void
SfbPhase1BuildModel (
  SFB_UINT8                   ActiveSlot,
  SFB_UINT8                   DefaultSlot,
  const SFB_PHASE1_SLOT_FACTS *SlotA,
  const SFB_PHASE1_SLOT_FACTS *SlotB,
  SFB_PHASE1_MODEL            *Model
  );

#endif /* __SUPER_FB_PHASE1_MODEL_H__ */
