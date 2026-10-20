# Architecture

## Boot flow

```text
authenticated vulnerable ABL
  -> raw efisp BDS.efi
  -> LinuxLoaderEntry
  -> FAT/ext4 boot-root discovery
  -> canoe.cfg + BCB policy
  -> original Canoe menu OR opt-in Phase 1 menu
  -> managed ABL loader / reboot request / resident Super Fastboot
```

`LinuxLoaderEntry` in `submodules/uefi/edk2/QcomModulePkg/Application/LinuxLoader/LinuxLoader.c` is the firmware application entry point. It enumerates partitions, starts Canoe's filesystem stack, mounts logging storage, clears stale launch evidence, disables the inherited watchdog, loads `canoe.cfg`, inspects Fastbootd and boot-once BCB state, applies the power-on policy, and dispatches either a menu/default or `FastbootInitialize`.

## Menu and input

The original UI is implemented by `SuperFbMenu.c`; `SuperFbMenuScaffold.c::SfbRunMenu` owns redraw, scrolling, timeout interruption, and key dispatch. `SfbDecodeMenuKey` maps Volume Up/Down to navigation and Qualcomm `SCAN_SUSPEND` (Power) or keyboard Enter to selection. Rendering uses UEFI `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` over the inherited console/framebuffer, with no Android, WebView, or JavaScript dependency.

`SuperFbPhase1Menu.c` is the opt-in five-row frontend. It reuses the scaffold, changes no low-level input code, validates Android rows through `SuperFbPhase1Model.c`, and calls only existing Canoe launch/reboot surfaces. The original menu is the default and fallback.

## Entry discovery and launching

`SuperFbEntries.c::SfbBuildMenu` combines validated `canoe.cfg` entries, known managed-loader probes, removable-media/BLS discovery, and built-in actions. Managed paths are `boot.efi`, `boot_a.efi`, `boot_b.efi`, and `boot_backup.efi`. `SfbLaunchEntryWithRetryPolicy` is the shared launch path; the legacy `SfbLaunchEntry` wrapper preserves upstream retry-refresh behavior. The path scopes security hooks, mode selection, sidecars, driver preloading, LoadOptions, logging, image load, and cleanup.

`SuperFbLaunchPolicy.c::SfbLoadMode2Profile` reads and parses the exact adjacent `.gm2p`. `SfbLoadTzMap` does the same for `.tzmap`. Phase 1 requires both to be present and valid before an Android row is enabled; it does not use the older launch-time `.tzmap` fallback as evidence of slot readiness.

The host-side `tools/canoe-bootmgr` crate validates/publishes prepared loader triplets and serializes the same configuration grammar. `PreparedLoader` validates ARM64 PE input, a 120-byte GM2P, and a 256-byte TZ map before publishing sidecars ahead of the loader.

## A/B metadata

`SuperFbSlots.c::SfbActiveSlot` reads `PART_ATT_ACTIVE_VAL` from both `abl_a` and `abl_b` in the already-enumerated GPT. It returns unknown unless exactly one complete slot is active. `SfbSlotRetries` reads successful, unbootable, and retry fields from those same GPT entries.

`SuperFbConfigEntrySlot` in `SuperFbEntries.c` interprets an entry ID/path suffix only as metadata. `SfbCheckConfigSlots` treats a saved-default mismatch as advisory in the original interface. Neither function switches slots.

The only implemented GPT mutation is `SfbResetActiveSlotRetry`. It intentionally accepts no slot argument and freshly targets only the slot already marked active. Vendor declarations such as `SetActiveSlot` have no definition in this tree. Therefore loading `boot_b.efi` does not establish coherent Slot B firmware/Android selection. Existing Canoe launches preserve their retry-refresh behavior; the Phase 1 menu uses the explicit no-refresh launch policy and remains read-only with respect to GPT.

## Recovery, Fastbootd, and Super Fastboot

`QcomModulePkg/Library/RebootTargetLib/RebootTargetLib.c::RebootTargetPrepare` reads `misc`, changes only the 32-byte `bootloader_message.command`, writes, flushes, and reads back. Recovery uses `boot-recovery`; Fastbootd uses `boot-fastboot`. Both perform a normal Qualcomm reset only after verified persistence. Fastbootd detection on the next boot preserves Canoe's optional Mode 2 override.

Super Fastboot is resident in this BDS: the menu returns an `EnterFastboot` decision and `LinuxLoaderEntry` enters `FastbootInitialize`. It does not write a reboot target.

## Configuration and recovery

`SuperFbConfig.c` is the pure firmware parser and editor. `SuperFbConfigStore.c` locates the boot root; `SuperFbConfigSave.c` performs recoverable publication with `canoe.cfg.prev`, temporary state, flush, and validation. The Rust implementation in `tools/canoe-bootmgr/src/config*` mirrors the grammar.

The additive `boot-manager-menu canoe|phase1` key defaults to `canoe`. Existing `menu-mode`, `key-window`, `menu-timeout`, `default`, Mode 0/1/2, `show-booting`, `fastbootd-mode2`, and `devinfo-repair` semantics remain intact.

`SuperFbBootOnce.c` continues to consume persisted selectors before power-on policy. With the Phase 1 frontend enabled, `SfbPhase1BootOnceSelectorReady` re-resolves an Android selector through the same active-slot and artifact gates, and the launch disables the GPT retry refresh. A rejected selector is consumed and opens the visible menu; the resident `fastboot` selector is unchanged.

## WebUI and KernelSU boundary

The inherited production browser/KernelSU manager lives in the separate upstream `canoe-boot-manager` project and enters this repository through explicit version-matched `CANOE_KSU_DIST` assets. It is not the firmware UI. This repository's `ui-prototype/` is a standalone visual simulator and is never packaged as UEFI behavior or granted device access.
