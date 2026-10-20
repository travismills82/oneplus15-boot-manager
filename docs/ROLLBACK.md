# Rollback

Phase 1 is opt-in and keeps the upstream UI compiled in.

## Configuration rollback

Set:

```text
boot-manager-menu canoe
```

or remove the additive key with an atomic config editor. Older upstream builds ignore the unknown key. Canoe's config store retains and validates `canoe.cfg.prev`; failed publication must not replace the last valid previous file.

## Source rollback

The untouched upstream baseline is preserved on `main` at `d8d45196c83a3d4c3e07c5efd73b4ceaf1b47a0d`. Experimental work lives only on `feature/grub-style-ab-menu`. Do not merge it to `main` before hardware qualification.

## Device rollback boundary

No hardware installation is part of Phase 1, so there is no Phase 1 device state to undo. A future device test must capture exact partition hashes, preserve a known-good BDS and boot-root configuration, establish both-slot recovery, and define read-back and power-loss checkpoints before any write. Never use these source-build instructions as authorization to flash.
