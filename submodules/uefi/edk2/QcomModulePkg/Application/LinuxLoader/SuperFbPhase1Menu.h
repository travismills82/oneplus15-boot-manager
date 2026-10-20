/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef __SUPER_FB_PHASE1_MENU_H__
#define __SUPER_FB_PHASE1_MENU_H__

#include "SuperFbMenu.h"

BOOLEAN
SfbRunPhase1BootMenu (
  IN SFB_BOOT_MODE InitialMode,
  IN BOOLEAN       AllowCountdown,
  IN BOOLEAN       FirstRun
  );

/* Attempt only a validated, currently active Android default. */
SFB_DEFAULT_RESULT
SfbLaunchPhase1Default (IN SFB_BOOT_MODE Mode);

/* Re-resolve a persisted Canoe selector against the same fail-closed slot and
 * sidecar gates used by the visible Phase 1 menu. */
BOOLEAN
SfbPhase1BootOnceSelectorReady (
  IN CONST CHAR8  *Selector,
  IN SFB_BOOT_MODE Mode
  );

#endif /* __SUPER_FB_PHASE1_MENU_H__ */
