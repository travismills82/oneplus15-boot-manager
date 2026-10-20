# Upstream baseline

This record was captured before Phase 1 source changes.

| Item | Recorded value |
| --- | --- |
| Upstream repository | `https://github.com/1vivy/gbl_root_canoe` |
| Upstream branch | `main` |
| Commit | `d8d45196c83a3d4c3e07c5efd73b4ceaf1b47a0d` |
| Commit date | `2026-10-16T19:08:07-06:00` |
| Commit subject | `Make BDS slot-label mismatch advisory-only` |
| `git describe` | `7.0.11-2-gd8d45196` |
| Canoe version | `7.0.11` |
| Canoe version code | `28` |

## Dependency identity

The repository contains no Git submodules; the EDK II tree is a squash-imported vendor subtree. `imports.toml` identifies that subtree as CodeLinaro ABL EDK II, upstream revision unknown, imported at local marker `2f5339b`, with Canoe patches maintained in-tree.

Recorded source/lock hashes:

| File | SHA-256 |
| --- | --- |
| `version.mk` | `7f0c2df75c5aea7e422d8d390f37b24a25df9b0cd1a57948f9b9ebac70d6263e` |
| `imports.toml` | `aace07739ea789b38639c5c8126ac0e0b7e49eca7a7a5d2e7bee0c537b8b72b0` |
| `tools/canoe-bootmgr/Cargo.lock` | `48b2daac00f8c4658720d971d672f5d4b633f7c8ef227516186bd35e0b6533a7` |
| `tools/mode2-profile/Cargo.lock` | `23a2ff8489df18882f1b09d22ab258527488511c7a4cc3e6610b6095486bd863` |
| `tools/abl-tzmap/Cargo.lock` | `0d6aebcc3a75603607b7c7635479b8b092951af0a4519f728b0ded80e37fa298` |
| `canoe-usbmsd.efi` | `e95493db65b19cf3107885dfc6e686e46d88c8e7795707f432026ec50e2e9622` |
| `canoe-managed-msd.efi` | `bdf5b6464436bd8e07fd16c184f2b2c66a2a343fa2d1975f944aadffb1708e12` |

The inherited workflow pins Rust `1.98.0`, builds on Ubuntu 22.04, and installs LLVM 20. The local baseline builder resolved `ubuntu:22.04` to `sha256:0a9887931b0e38af2e3487ed39e29436b0535c4f3f52e57745baaf79d1898956`; the locally built `gbl_builder:latest` image ID was `sha256:24270f4830ab7b77d234c030a0c306d59231e71d64fa5be3449c05925a3965d2`.

## Baseline verification

Before modification, `make test` passed and a clean Docker build of `submodules/uefi` produced `BDS.efi` plus all nine inherited Android EFI tools. This proves host behavior and compilation only; it is not device-boot evidence.

## Remotes and history policy

- `origin`: `https://github.com/travismills82/oneplus15-boot-manager.git`
- `upstream`: `https://github.com/1vivy/gbl_root_canoe.git`
- `main` is the untouched upstream baseline.
- Phase 1 work is restricted to `feature/grub-style-ab-menu`.
- Upstream history and tags are retained. No force-push is permitted.
