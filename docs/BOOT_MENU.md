# Phase 1 boot menu

The native menu is implemented in `SuperFbPhase1Menu.c` and uses Canoe's UEFI console/input scaffold. Its title is `ONEPLUS 15 BOOT MANAGER`, the selection is white on blue, and the remaining background is black. Each row shows availability plus active/default markers; the focused row exposes a compact description and refusal reason.

## Entries

| ID | Type | Required data/action | Phase 1 rule |
| --- | --- | --- | --- |
| `android-a` | Android | `boot_a.efi`, `.gm2p`, `.tzmap`, Slot A GPT retry state | Enabled only when A is already active and viable |
| `android-b` | Android | `boot_b.efi`, `.gm2p`, `.tzmap`, Slot B GPT retry state | Enabled only when B is already active and viable |
| `recovery` | Recovery | one-time `boot-recovery` in `misc` | Standard BCB write/read-back, then reset |
| `super-fastboot` | Resident firmware | Canoe `FastbootLib` | Returns to the existing Super Fastboot loop |
| `fastbootd` | Userspace Fastboot | one-time `boot-fastboot` in `misc` | Standard BCB write/read-back, then reset |

Volume Up/Down moves with wraparound. Power selects. Any navigation cancels the current countdown. Unavailable rows remain focusable so the exact reason is visible, but selection performs no action.

## Configuration

The Phase 1 frontend is deliberately opt-in:

```text
version 1
generation 1
boot-manager-menu phase1
menu-mode menu
key-window 1200
menu-timeout 5
show-booting yes
fastbootd-mode2 yes
default android-a
mode 1
devinfo-repair asneeded

entry android-a
  title Android Slot A
  image boot_a.efi
  mode 1
  role other

entry android-b
  title Android Slot B
  image boot_b.efi
  mode 1
  role other
```

`default` controls menu focus/countdown only. Phase 1 launches it unattended only if it resolves to the currently active, fully validated Android row. It uses the no-retry-refresh launch path and never changes GPT active-slot metadata. `menu-timeout 0` disables the countdown. `menu-mode menu` is the always-show-menu policy; `silent` retains the upstream Volume Up escape window.

Canoe's persisted one-time selector remains supported. Under Phase 1 it is consumed atomically, re-resolved through the same active-slot/image/sidecar/retry checks, and uses the no-retry-refresh launch path. A missing, inactive, or invalid selector opens the menu instead of booting another row.

Set `boot-manager-menu canoe` or omit the key to use the original Canoe menu. Invalid values are rejected and fail back to Canoe.

## Browser correspondence

The standalone [browser prototype](../ui-prototype/README.md) mirrors the five-entry model, navigation, default focus, countdown, active-slot status, availability reasons, theme choices, and simulated transitions. It shares concepts, not runtime code, with UEFI and cannot perform a boot or write.
