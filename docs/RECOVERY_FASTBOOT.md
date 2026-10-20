# Recovery and fastboot paths

## Recovery

The Phase 1 Recovery row calls `RebootTargetPrepare(RebootTargetRecovery)`. The shared library writes the AOSP `boot-recovery` command to the first 32 bytes of `misc`, preserves the remainder of the block, flushes, reads back, and returns the Qualcomm normal-reset reason. The firmware resets only after success. This is a one-time request and does not change the saved Android default.

## Fastbootd

The Fastbootd row uses the same transaction with `RebootTargetFastbootd`, which writes `boot-fastboot`. On the next BDS pass, `RebootTargetIsFastbootd` detects the command. Existing `fastbootd-mode2 yes|no` behavior remains authoritative; Phase 1 does not silently change Mode 2 policy.

## CANOE Super Fastboot

Super Fastboot is not Android bootloader fastboot and does not require a BCB write. The Phase 1 frontend exits with `EnterFastboot`, after which `LinuxLoaderEntry` publishes the current session mode and enters Canoe's existing `FastbootInitialize` loop.

## Failure behavior

Missing/ambiguous `misc`, unsupported block geometry, write errors, flush errors, or read-back mismatch keep the user in the menu and display an error. The firmware does not reset after a failed request. No Recovery/Fastbootd path changes the GPT active slot.
