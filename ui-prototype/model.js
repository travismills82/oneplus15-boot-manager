export const ENTRY_DEFINITIONS = Object.freeze([
  { id: "android-a", label: "Boot Android — Slot A", description: "Launch the validated Slot A loader and matching sidecars.", type: "android", slot: "a", required: "boot_a.efi + .gm2p + .tzmap" },
  { id: "android-b", label: "Boot Android — Slot B", description: "Launch the validated Slot B loader and matching sidecars.", type: "android", slot: "b", required: "boot_b.efi + .gm2p + .tzmap" },
  { id: "recovery", label: "Boot Recovery", description: "Set a one-time recovery request without changing the default OS.", type: "recovery", slot: null, required: "misc BCB: boot-recovery" },
  { id: "super-fastboot", label: "Boot Super Fastboot", description: "Enter CANOE-BDS resident Super Fastboot.", type: "super-fastboot", slot: null, required: "CANOE FastbootLib" },
  { id: "fastbootd", label: "Boot Fastbootd", description: "Set a one-time userspace Fastboot request through recovery.", type: "fastbootd", slot: null, required: "misc BCB: boot-fastboot" }
]);

export function buildEntries({ activeSlot = "a", defaultId = "android-a", loaders = {} } = {}) {
  return ENTRY_DEFINITIONS.map((entry) => {
    let available = true;
    let reason = "Available";
    if (entry.slot) {
      const loader = loaders[entry.slot] ?? { image: true, gm2p: true, tzmap: true, retryKnown: true, successful: false, retries: 3, unbootable: false };
      if (!["a", "b"].includes(activeSlot)) [available, reason] = [false, "Active slot is unknown"];
      else if (entry.slot !== activeSlot) [available, reason] = [false, "Inactive-slot switching is not verified in Phase 1"];
      else if (!loader.image) [available, reason] = [false, "Prepared loader is missing"];
      else if (loader.imageValid === false) [available, reason] = [false, "Loader is not an AArch64 UEFI application"];
      else if (!loader.gm2p) [available, reason] = [false, "Matching .gm2p is missing or invalid"];
      else if (!loader.tzmap) [available, reason] = [false, "Matching .tzmap is missing or invalid"];
      else if (!loader.retryKnown) [available, reason] = [false, "Slot retry metadata is unavailable"];
      else if (loader.unbootable) [available, reason] = [false, "GPT marks this slot unbootable"];
      else if (!loader.successful && loader.retries === 0) [available, reason] = [false, "No slot retries remain"];
    }
    return { ...entry, active: entry.slot === activeSlot, default: Boolean(entry.slot) && entry.id === defaultId && available, available, reason };
  });
}

export function moveSelection(index, direction, count = ENTRY_DEFINITIONS.length) {
  if (count < 1) return 0;
  return (index + (direction < 0 ? -1 : 1) + count) % count;
}

export function tickCountdown(state, elapsedSeconds) {
  if (state.interrupted || state.remaining <= 0) return { ...state };
  return { ...state, remaining: Math.max(0, state.remaining - elapsedSeconds) };
}
