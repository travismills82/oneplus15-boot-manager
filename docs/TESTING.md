# Testing

> All current outputs are **EXPERIMENTAL — NOT VALIDATED ON HARDWARE**.

## Host-side gates

- `make test`: aggregate Canoe host suite, including config recovery, BCB/reboot handling, menu navigation/countdown, slot metadata, launch cleanup, filesystem parsers, patcher, targets, and Rust tools.
- `make -C submodules/uefi/tests test`: UEFI host contracts. `test_phase1_model` specifically covers entry order, active/inactive gating, unknown slots, missing/invalid images, invalid sidecars, missing retry data, unbootable state, retry exhaustion, successful-slot behavior, and default refusal.
- `cargo test --locked --manifest-path tools/canoe-bootmgr/Cargo.toml`: configuration parser, serializer, CLI, atomic recovery, and artifact publication.
- `npm test` in `ui-prototype/`: five-entry model, navigation wrap, countdown interruption, inactive-slot refusal, and sidecar failures.
- `npm run build` in `ui-prototype/`: produces a static `dist/` without network or device code.

## Firmware build

The canonical build is:

```sh
docker build -t gbl_builder - < Dockerfile
docker run --rm -v "$PWD:/workspace" -w /workspace gbl_builder \
  bash -lc 'make -C submodules/uefi clean && make -C submodules/uefi build && make -C submodules/uefi tools'
```

The workflow archives `BDS.efi`, inherited standalone EFI tools, a warning marker, and `SHA256SUMS`. Artifact generation never flashes or boots them.

## Coverage mapping

| Requested behavior | Evidence layer |
| --- | --- |
| Config parsing and recovery | C parser/store tests plus Rust config tests |
| Five entries and availability | pure Phase 1 C model plus browser model |
| Up/Down/Power and countdown | inherited scaffold tests plus prototype tests |
| Default and one-time selection | C config/model/boot-once tests and Rust round-trip tests |
| Slot detection/retry state | existing GPT slot tests plus Phase 1 model |
| Missing image/sidecars | loader artifact tests plus Phase 1 model |
| Recovery/Fastbootd request | existing `RebootTargetLib` host tests |
| Interrupted config/transition | atomic config recovery tests; no GPT transition exists |
| Original-menu fallback | default parser value and frontend routing |

## Explicit limit

A successful test/build proves neither that the OnePlus 15 framebuffer renders as expected nor that the selected loader boots the device. No phone was flashed, rebooted, unlocked, or modified during Phase 1 development. Physical testing requires a separate reviewed plan and explicit authorization.
