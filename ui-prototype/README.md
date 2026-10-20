# Browser UI prototype

This directory is a standalone, buildless simulation of the Phase 1 UEFI menu. It never calls ADB, fastboot, WebUSB, WebView, Android APIs, or any partition interface.

Run `npm test` and `npm run build`, then serve `dist/` with any static HTTP server. Keyboard Up/Down mirrors volume navigation and Enter mirrors the Power-button selection used by the firmware.

The shared concepts correspond directly to firmware:

| Prototype | Native UEFI implementation |
| --- | --- |
| `model.js` entry definitions and fail-closed facts | `SuperFbPhase1Model.c` |
| highlighted row and keyboard handler | `SfbRunMenu` plus `SuperFbPhase1Menu.c` |
| countdown/default controls | `menu-timeout`, `menu-mode`, and configured `default` |
| Recovery/Fastbootd simulation | `RebootTargetPrepare` BCB command path |
| Super Fastboot simulation | existing CANOE-BDS `FastbootInitialize` path |

Changing the simulated active slot demonstrates menu state only. The firmware does not implement slot switching in Phase 1.
