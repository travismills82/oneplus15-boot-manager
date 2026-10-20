# OnePlus 15 Boot Manager

> **EXPERIMENTAL — NOT VALIDATED ON HARDWARE**

A GRUB-style native UEFI boot selector for the OnePlus 15 (Qualcomm SM8850), built as a full-history fork of [GBL Root Canoe](https://github.com/1vivy/gbl_root_canoe). This repository is independent of the OxygenOS and LineageOS kernel repositories.

Phase 1 provides five menu destinations:

- Android — Slot A
- Android — Slot B
- Recovery
- CANOE Super Fastboot
- Android userspace Fastbootd

This is an A/B selector experiment, not independent dual-boot support. It does not flash images, change the bootloader lock state, mark slots successful, or implement a verified GPT slot transition. Only the slot already marked active by the SM8850 boot chain can become available, and only after its prepared loader, `.gm2p`, `.tzmap`, and retry metadata pass validation. The inactive Android row stays visible and disabled.

## Status

- Native UEFI menu: implemented behind opt-in `boot-manager-menu phase1`.
- Original Canoe menu: retained and remains the default (`boot-manager-menu canoe`).
- Browser prototype: [ui-prototype](ui-prototype/README.md), simulation only.
- Host policy/config tests: implemented.
- AArch64 UEFI build: supported by the inherited Canoe Docker toolchain.
- Phone flashing and hardware validation: explicitly out of scope for this phase.

## Quick checks

```sh
make test
docker build -t gbl_builder - < Dockerfile
docker run --rm -v "$PWD:/workspace" -w /workspace gbl_builder \
  bash -lc 'make -C submodules/uefi clean && make -C submodules/uefi build && make -C submodules/uefi tools'
docker run --rm -v "$PWD/ui-prototype:/app" -w /app node:22-alpine \
  sh -lc 'npm test && npm run build'
```

No command above accesses a phone.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Boot menu and configuration](docs/BOOT_MENU.md)
- [A/B slot safety](docs/AB_SLOT_SWITCHING.md)
- [Recovery and fastboot paths](docs/RECOVERY_FASTBOOT.md)
- [Testing evidence and limits](docs/TESTING.md)
- [Rollback](docs/ROLLBACK.md)
- [Upstream baseline and dependency record](docs/UPSTREAM.md)

The original Canoe operator and implementation documentation remains under [`wiki/`](wiki/) and the historical reconstruction material remains under [`docs/`](docs/).

## License and attribution

The complete upstream Git history, copyright notices, contributor records, and GPL-2.0-or-later [`LICENSE`](LICENSE) are preserved. New firmware files use the license style of their containing Canoe/Qualcomm source area. See [UPSTREAM.md](docs/UPSTREAM.md).
