import test from "node:test";
import assert from "node:assert/strict";
import { buildEntries, moveSelection, tickCountdown } from "../model.js";

test("the model exposes exactly the five Phase 1 entries", () => {
  assert.deepEqual(buildEntries().map(({ id }) => id), ["android-a", "android-b", "recovery", "super-fastboot", "fastbootd"]);
});

test("inactive slots remain visible but fail closed", () => {
  const entries = buildEntries({ activeSlot: "a", defaultId: "android-b" });
  assert.equal(entries[0].available, true);
  assert.equal(entries[1].available, false);
  assert.match(entries[1].reason, /not verified/);
  assert.equal(entries[1].default, false);
});

test("missing and mismatched sidecars disable the active slot", () => {
  let entries = buildEntries({ loaders: { a: { image: true, imageValid: false, gm2p: true, tzmap: true, retryKnown: true, retries: 3 } } });
  assert.match(entries[0].reason, /AArch64 UEFI/);
  entries = buildEntries({ loaders: { a: { image: true, gm2p: false, tzmap: true, retryKnown: true, retries: 3 } } });
  assert.match(entries[0].reason, /gm2p/);
  entries = buildEntries({ loaders: { a: { image: true, gm2p: true, tzmap: false, retryKnown: true, retries: 3 } } });
  assert.match(entries[0].reason, /tzmap/);
});

test("navigation wraps and input interrupts the countdown", () => {
  assert.equal(moveSelection(0, -1), 4);
  assert.equal(moveSelection(4, 1), 0);
  assert.deepEqual(tickCountdown({ remaining: 3, interrupted: false }, 1), { remaining: 2, interrupted: false });
  assert.deepEqual(tickCountdown({ remaining: 3, interrupted: true }, 1), { remaining: 3, interrupted: true });
});
