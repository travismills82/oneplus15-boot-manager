# A/B slot switching assessment

## Finding

Phase 1 does **not** implement slot switching.

On this SM8850 tree, the active chain is inferred from the `PART_ATT_ACTIVE_VAL` attributes on `abl_a`/`abl_b`. Android boot-success, retry, and unbootable fields are read from the same GPT entries. A menu entry named `android-b` or a file named `boot_b.efi` does not switch ABL, boot, init_boot, vendor_boot, vbmeta, modem, or other slot-suffixed firmware partitions.

The source tree contains declarations for vendor slot helpers but no implementations. The one available GPT writer, `SfbResetActiveSlotRetry`, can only reset the freshly observed active slot and intentionally has no target-slot parameter. It is not a slot switch. Canoe's original launch paths retain that legacy retry refresh; Phase 1 calls `SfbLaunchEntryWithRetryPolicy(..., FALSE)` so selecting Android through the new menu does not invoke it.

## Phase 1 gate

An Android row is available only when all conditions hold:

1. exactly one active slot is established from `abl_a`/`abl_b`;
2. the row represents that already-active slot;
3. the exact prepared loader is present on the internal boot root;
4. it parses as a PE32+ AArch64 UEFI application;
5. its adjacent `.gm2p` parses;
6. its adjacent `.tzmap` parses;
7. retry metadata is readable;
8. the slot is not unbootable; and
9. the slot is successful or has at least one retry remaining.

The inactive row remains unavailable with the reason `inactive-slot switching is not verified in Phase 1`. A configured default for that row is not honored. A stored Canoe boot-once selector is re-resolved through the same gate and launches only a validated, already-active Android row, without refreshing GPT retry metadata. Invalid or inactive selectors are consumed and fall back to the menu. Resident Super Fastboot remains allowed.

No Android selection in Phase 1 writes GPT metadata, boot, init_boot, vendor_boot, vbmeta, ABL, or another partition. No code marks an untested slot successful. Recovery and Fastbootd remain explicit one-time actions and write only the established BCB command in `misc`, with flush and read-back before reset.

## Work required before switching

A later phase needs device-specific evidence for the complete SM8850 slot-selection contract, including priority/tries/success transitions, firmware partition coherence, bootloader rollback behavior, and recovery from power loss at every write boundary. It needs an independently reviewed, atomic transition design and controlled physical qualification with a known-good rollback path. Until then, the inactive slot must remain disabled.
