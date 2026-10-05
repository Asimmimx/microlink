# Changelog

## v2.2.0: maintained fork (2026-10-05)

First release of [Asimmimx/microlink](https://github.com/Asimmimx/microlink), a maintained fork of [CamM2325/microlink](https://github.com/CamM2325/microlink) v2.1.0.

Every fix below was checked on real hardware (ESP32-C3 without PSRAM, ESP-IDF v5.3.2), usually by reproducing the bug first and then measuring before and after. Each one is a separate commit. Upstream pull requests were not merged; where one existed, the bug was reproduced here and the fix written and tested independently.

### Fixes

| Problem | Before | After |
|---|---|---|
| Single-core chips (C3, C6) | Reboot loop on start (task pinned to Core 1) | Connects normally |
| Boards without PSRAM | `MapRequest failed` forever (out of memory) | Peer list loads |
| Crash when a packet arrived while a socket was opening ([#17](https://github.com/CamM2325/microlink/issues/17)) | Load access fault in `udp_input`, reboot | lwIP is only touched from its own thread. Checked with `CONFIG_LWIP_CHECK_THREAD_SAFETY` |
| Peak RAM while joining a tailnet | Free RAM dipped to 9.7 KB on a C3 | Lowest point 84–93 KB; still joins with only 185 KB free at boot |
| Long-poll allocated 64 KB per message | Control connection lost when RAM was tight | Stays connected |
| FreeRTOS at 100 Hz, the ESP-IDF default ([#36](https://github.com/CamM2325/microlink/issues/36)) | Watchdog every 5 s; app task never ran | No watchdog trips |
| Packet loss under load (as in PRs [#30](https://github.com/CamM2325/microlink/pull/30), [#32](https://github.com/CamM2325/microlink/pull/32), [#38](https://github.com/CamM2325/microlink/pull/38)) | Per-packet INFO logs throttled the tunnel: 50 msg/s gave 71% loss and 784 ms | 100 msg/s: 0% loss, 19 ms |
| DERP relay fixed to Dallas ([#19](https://github.com/CamM2325/microlink/issues/19)) | Relay latency from Turkey: 168 ms | Closest region measured and remembered: about 60 ms |
| Failed DERP connects ([#37](https://github.com/CamM2325/microlink/pull/37)) | ~10 KB leaked per attempt; gave up after 3 attempts | No leak; retries with 2–60 s backoff |
| Peer online status ([#24](https://github.com/CamM2325/microlink/pull/24)) | Every peer reported online | Matches `tailscale status` |
| UDP RX task priority ([#39](https://github.com/CamM2325/microlink/pull/39)) | Same as ESP-IDF's reserved Bluetooth controller priority | 12, below the system tasks; same throughput |
| Tunnel MTU ([#34](https://github.com/CamM2325/microlink/issues/34)) | 1420 (plain WireGuard) | 1280 (Tailscale) |
| Web config panel on the original ESP32 | Build error (no temperature sensor) | Builds |

### Setup

- One component folder. The `wireguard_lwip` symlink that broke every Windows build is replaced by `idf_component.yml`.
- `sdkconfig.credentials` is actually read now (it was documented but ignored).
- `basic_connect` builds for any chip with `idf.py set-target <chip>`. Buffer sizes follow PSRAM automatically.
- `AGENTS.md` lets an AI assistant set the project up end to end. Two helper scripts support it: `tools/serial_log.py` and `tools/udp_echo.py`.

### Behaviour changes to know about

- **PreferredDERP**: the device now reports the region it measured as closest, not region 9. It is stored in NVS under the key `derp_pref`.
- **`microlink_peer_info_t.online`** now means "online according to the control plane". Before, it meant "has a slot in the local peer table".
- **Logs**: per-packet and per-tick lines (`UDP RX`, `WG RX/TX`, `DISCO ...`, `DERP status`, `HEARTBEAT`) are now DEBUG. Set the `ml_*` tags to DEBUG to see them again.
- **Defaults without PSRAM**: `ML_H2_BUFFER_SIZE_KB` 80 (a cap, not a reservation), `ML_JSON_BUFFER_SIZE_KB` 64 (no longer used for the initial MapResponse), `ML_MAX_PEERS` 8.

### Still open

- Unvalidated endpoints for peers behind CGNAT ([#18](https://github.com/CamM2325/microlink/issues/18)) and stale endpoints for peers reached through DERP ([#41](https://github.com/CamM2325/microlink/issues/41)). Testing these needs a phone on mobile data.
- Registration errors are silent ([#23](https://github.com/CamM2325/microlink/pull/23)).
- RST_STREAM/GOAWAY on the long-poll are not handled ([#40](https://github.com/CamM2325/microlink/pull/40)).
