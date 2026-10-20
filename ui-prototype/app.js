import { ENTRY_DEFINITIONS, buildEntries, moveSelection } from "./model.js";

const elements = {
  entries: document.querySelector("#entries"), countdown: document.querySelector("#countdown"),
  activeSlot: document.querySelector("#active-slot"), activeLabel: document.querySelector("#active-slot-label"),
  defaultEntry: document.querySelector("#default-entry"), timeout: document.querySelector("#timeout"),
  timeoutValue: document.querySelector("#timeout-value"), theme: document.querySelector("#theme"),
  restart: document.querySelector("#restart"), transition: document.querySelector("#transition"),
  transitionTitle: document.querySelector("#transition-title"), transitionDetail: document.querySelector("#transition-detail"),
  returnButton: document.querySelector("#return")
};

for (const entry of ENTRY_DEFINITIONS.filter(({ slot }) => slot)) elements.defaultEntry.add(new Option(entry.label, entry.id));
const state = { selected: 0, activeSlot: "a", defaultId: "android-a", timeout: 5, remaining: 5, interrupted: false, timer: null };

function entries() { return buildEntries({ activeSlot: state.activeSlot, defaultId: state.defaultId }); }
function stopCountdown() { state.interrupted = true; clearInterval(state.timer); renderCountdown(); }
function renderCountdown() {
  const selected = entries()[state.selected];
  elements.countdown.textContent = state.interrupted || state.timeout === 0 || !selected.default
    ? "Countdown paused — choose an entry"
    : `Default entry will start in ${state.remaining} second${state.remaining === 1 ? "" : "s"}`;
}
function render() {
  const current = entries();
  elements.activeLabel.textContent = state.activeSlot === "unknown" ? "?" : state.activeSlot.toUpperCase();
  elements.entries.replaceChildren(...current.map((entry, index) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `entry${index === state.selected ? " selected" : ""}${entry.available ? "" : " unavailable"}`;
    button.role = "option"; button.ariaSelected = String(index === state.selected); button.dataset.index = index;
    const badges = [entry.active && "ACTIVE", entry.default && "DEFAULT", !entry.available && "UNAVAILABLE"].filter(Boolean);
    button.innerHTML = `<span class="index">0${index + 1}</span><span class="copy"><strong>${entry.label}</strong><small>${entry.available ? entry.description : entry.reason}</small></span><span class="badges">${badges.map((badge) => `<span class="badge${badge === "UNAVAILABLE" ? " dim" : ""}">${badge}</span>`).join("")}</span>`;
    button.addEventListener("click", () => { stopCountdown(); state.selected = index; render(); selectEntry(); });
    return button;
  }));
  renderCountdown();
}
function selectEntry() {
  const entry = entries()[state.selected];
  if (!entry.available) { elements.countdown.textContent = entry.reason; return; }
  elements.transitionTitle.textContent = entry.label;
  elements.transitionDetail.textContent = `${entry.description} Required data: ${entry.required}. This is a visual simulation; no boot request or device write occurred.`;
  elements.transition.showModal();
}
function startCountdown() {
  clearInterval(state.timer); state.remaining = state.timeout;
  state.interrupted = state.timeout === 0 || !entries()[state.selected].default;
  renderCountdown();
  if (state.interrupted) return;
  state.timer = setInterval(() => {
    if (state.interrupted) return;
    state.remaining -= 1; renderCountdown();
    if (state.remaining <= 0) { clearInterval(state.timer); selectEntry(); }
  }, 1000);
}

document.addEventListener("keydown", (event) => {
  if (elements.transition.open) return;
  if (!["ArrowUp", "ArrowDown", "Enter"].includes(event.key)) return;
  event.preventDefault(); stopCountdown();
  if (event.key === "Enter") selectEntry();
  else { state.selected = moveSelection(state.selected, event.key === "ArrowUp" ? -1 : 1); render(); }
});
elements.activeSlot.addEventListener("change", () => { state.activeSlot = elements.activeSlot.value; stopCountdown(); render(); });
elements.defaultEntry.addEventListener("change", () => { state.defaultId = elements.defaultEntry.value; state.selected = ENTRY_DEFINITIONS.findIndex(({ id }) => id === state.defaultId); startCountdown(); render(); });
elements.timeout.addEventListener("input", () => { state.timeout = Number(elements.timeout.value); elements.timeoutValue.textContent = `${state.timeout} second${state.timeout === 1 ? "" : "s"}`; startCountdown(); });
elements.theme.addEventListener("change", () => { document.documentElement.dataset.theme = elements.theme.value; });
elements.restart.addEventListener("click", () => { state.selected = Math.max(0, ENTRY_DEFINITIONS.findIndex(({ id }) => id === state.defaultId)); startCountdown(); render(); });
elements.returnButton.addEventListener("click", () => { elements.transition.close(); startCountdown(); });
elements.transition.addEventListener("cancel", (event) => { event.preventDefault(); elements.transition.close(); startCountdown(); });

render(); startCountdown();
