# MicroLink: Tailscale for ESP32

Put an ESP32 on your [Tailscale](https://tailscale.com) network. Once it's on your tailnet, any of your devices can reach it from anywhere: phone, laptop, server. You don't need port forwarding or a public IP.

> This is a maintained fork of [CamM2325/microlink](https://github.com/CamM2325/microlink), which has had no commits since March 2026.
> It adds bug fixes, plus easier setup and ESP32-C3 support. Every fix was tested on real hardware ([what changed](#what-this-fork-fixes)).

---

## Which ESP32 works?

<img src="docs/compatibility.svg" alt="ESP32 compatibility chart: ESP32-C3 tested; ESP32-S3, ESP32 and ESP32-C6 should work; ESP32-S2 and C2 have too little RAM without PSRAM; ESP32-C5/C61 need a newer ESP-IDF; ESP32-H2 and P4 are not supported" width="760">

<details>
<summary>Same table as text</summary>

| Chip | Status | Notes |
|---|---|---|
| ESP32-C3 | ✅ Tested on hardware | Tested with this fork: no PSRAM, single core |
| ESP32-S3 | 🟡 Should work | The original author's main board. PSRAM recommended |
| ESP32 | 🟡 Should work | Tested by the original author (no PSRAM) |
| ESP32-C6 | 🟡 Should work | Builds. Single core like the C3, with more RAM |
| ESP32-S2 | 🟠 Low RAM | Builds. RAM use not measured on an S2 yet; use a board with PSRAM (most S2 boards have it) |
| ESP32-C2 | 🟠 Low RAM | Builds. MicroLink needs about 120–140 KB free after WiFi connects; a C2 has about 95 KB by default ([Espressif](https://developer.espressif.com/blog/2025/11/esp32c2-ram-optimization/)) |
| ESP32-C5 / C61 | ❔ Unknown | WiFi support needs ESP-IDF 5.5+. Not built yet |
| ESP32-H2 | ❌ No | No WiFi (Thread/Zigbee/BLE only) |
| ESP32-P4 | ❌ No | No built-in WiFi |

"Should work" means it builds with ESP-IDF v5.3.2 using this fork, but hasn't been run on hardware with this fork yet. The RAM numbers come from an ESP32-C3 with part of its memory held back to mimic a smaller chip ([details](#what-this-fork-fixes)). If you try one, please [open an issue](https://github.com/Asimmimx/microlink/issues) with the result.

</details>

---

## Set it up with an AI assistant

Working with Claude Code, Codex, Cursor or Copilot? Open this repository and say:

> *Set up MicroLink on my ESP32 board.*

The assistant reads [`AGENTS.md`](AGENTS.md). It detects your chip and serial port, asks you only for your WiFi details and Tailscale key, then builds, flashes and checks the connection.

---

## Quick start (about 15 minutes)

**You need:** an ESP32 board from the table above, a USB cable, and a free Tailscale account.

### 1. Install ESP-IDF

Follow Espressif's installer for your OS (v5.3 or newer):
[Windows](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html) ·
[Linux / macOS](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html)

Run the commands below in the **ESP-IDF terminal** that the installer sets up.

### 2. Download MicroLink

```bash
git clone https://github.com/Asimmimx/microlink.git
cd microlink/examples/basic_connect
```

### 3. Get a Tailscale auth key

Open **[Tailscale admin → Settings → Keys](https://login.tailscale.com/admin/settings/keys)** and click **Generate auth key**:

- **Reusable**: off
- **Ephemeral**: **off** (otherwise the device disappears when it powers off)
- **Pre-approved**: on (if you see the option)

Copy the key. It starts with `tskey-auth-`.

### 4. Add your WiFi and key

Copy the template:

```bash
cp sdkconfig.credentials.example sdkconfig.credentials
```

Open `sdkconfig.credentials` in any text editor and fill in the three lines:

```ini
CONFIG_ML_WIFI_SSID="your-wifi-name"
CONFIG_ML_WIFI_PASSWORD="your-wifi-password"
CONFIG_ML_TAILSCALE_AUTH_KEY="tskey-auth-..."
```

`sdkconfig.credentials` is git-ignored, so your secrets stay local.

### 5. Build and flash

Replace `esp32c3` with your chip and `COM5` with your board's port (`/dev/ttyUSB0` on Linux, `/dev/cu.usbserial-*` on macOS):

```bash
idf.py set-target esp32c3
idf.py -p COM5 flash monitor
```

After about 15 seconds you should see:

```
MicroLink state: CONNECTED
Connected! VPN IP: 100.x.y.z
```

The device now shows up in your [Tailscale machines list](https://login.tailscale.com/admin/machines). Click **"…" → Disable key expiry** there, or it will drop off the network after 180 days.

### 6. Try it

From any computer on your tailnet:

```bash
ping 100.x.y.z
echo "hello" | nc -u 100.x.y.z 9000      # the example echoes it back
```

---

## Use it in your own project

1. Copy [`examples/basic_connect/sdkconfig.defaults`](examples/basic_connect/sdkconfig.defaults) into your project. It enables the crypto and network options MicroLink needs.
2. Add one line to your project's top-level `CMakeLists.txt`, before `include(...project.cmake)`:

   ```cmake
   set(EXTRA_COMPONENT_DIRS "path/to/microlink/components/microlink")
   ```

3. Start MicroLink once WiFi is connected:

   ```c
   #include "microlink.h"

   microlink_config_t cfg = {
       .auth_key    = "tskey-auth-...",
       .device_name = "my-esp32",
       .enable_derp = true, .enable_stun = true, .enable_disco = true,
   };
   microlink_t *ml = microlink_init(&cfg);
   microlink_start(ml);

   /* once microlink_is_connected(ml) is true: */
   microlink_udp_socket_t *sock = microlink_udp_create(ml, 9000);
   microlink_udp_send(sock, microlink_parse_ip("100.64.0.5"), 9000, "hi", 2);
   ```

[`examples/basic_connect/main/main.c`](examples/basic_connect/main/main.c) is a complete, working program. The full API is in the [reference](docs/REFERENCE.md#api-reference).

---

## What this fork fixes

Each fix is a separate commit, and each one was checked on hardware before and after the change.

| Problem | Before | After |
|---|---|---|
| Single-core chips (C3/C6/H2) | Reboot loop on start | Connects normally |
| Boards without PSRAM | `MapRequest failed` forever (out of memory) | Peer list loads (80 KB buffer) |
| Long-poll took 64 KB per message | Control connection lost when RAM is tight | Stays connected |
| FreeRTOS at 100 Hz (ESP-IDF default) | Watchdog every 5 s, app never runs ([#36](https://github.com/CamM2325/microlink/issues/36)) | No watchdog trips |
| Tunnel MTU 1420 instead of 1280 | Oversized packets to some peers ([#34](https://github.com/CamM2325/microlink/issues/34)) | Uses Tailscale's 1280 |
| Crash when a packet arrived while a socket was opening ([#17](https://github.com/CamM2325/microlink/issues/17)) | Load access fault in `udp_input`, device reboots | lwIP is only touched from its own thread. Verified with ESP-IDF's `LWIP_CHECK_THREAD_SAFETY` |
| Peak RAM while joining the tailnet | On an ESP32-C3, free RAM dipped to 9.7 KB | Lowest point 86–93 KB; still connects with only 185 KB free at boot. Unused JSON fields dropped, DERP map parsed one region at a time, exact-size buffer |
| Web config panel on the original ESP32 | Build error (no temperature sensor) | Builds; the temperature shows as empty |
| Packet loss under load ([#30](https://github.com/CamM2325/microlink/pull/30), [#32](https://github.com/CamM2325/microlink/pull/32), [#38](https://github.com/CamM2325/microlink/pull/38)) | Every packet logged at INFO, so the UART throttled the tunnel. 50 msg/s: 71% loss, 784 ms | Per-packet logs at DEBUG, deeper RX queue. 100 msg/s: 0% loss, 19 ms |
| DERP relay hardcoded to Dallas ([#19](https://github.com/CamM2325/microlink/issues/19)) | Relay latency from Turkey 168 ms | Closest region picked by measurement and remembered (Nuremberg/Warsaw, about 60 ms) |
| Failed DERP connects ([#37](https://github.com/CamM2325/microlink/pull/37)) | ~10 KB leaked per failed attempt, and no retry after 3 failures | No leak; retries with backoff until connected |
| Peer "online" status ([#24](https://github.com/CamM2325/microlink/pull/24)) | Every peer reported online | Matches `tailscale status` |
| UDP RX task priority ([#39](https://github.com/CamM2325/microlink/pull/39)) | Shared ESP-IDF's reserved Bluetooth controller priority | Below the system tasks; same throughput |
| Setup | `sdkconfig.credentials` was never read. The Windows build broke on a symlink. The example only built for ESP32-S3 | Credentials file works, Windows builds, and `set-target` works for any chip |

Upstream fixes are rewritten and checked here, not merged blind; only what could be reproduced or measured on hardware is in. Still open: unvalidated endpoints for peers behind CGNAT ([#18](https://github.com/CamM2325/microlink/issues/18)) and stale endpoints for peers reached through DERP ([#41](https://github.com/CamM2325/microlink/issues/41)), which need a phone on mobile data to test; registration errors being silent ([#23](https://github.com/CamM2325/microlink/pull/23)); and RST_STREAM/GOAWAY on the long-poll ([#40](https://github.com/CamM2325/microlink/pull/40)).

---

## Troubleshooting

| Symptom | Fix |
|---|---|
| Device never shows up in Tailscale | The auth key is wrong or already used. Make a new one, and erase the old identity with `idf.py -p PORT erase-flash` |
| Changed `sdkconfig.credentials` but nothing changed | Delete the `sdkconfig` file and build again. Settings are only copied in on the first build |
| First message right after boot gets no reply | Normal. The first tunnel takes about 15 s to set up |
| `Failed to resolve component 'wireguard_lwip'` | You're on the old upstream repo. Use this fork |

More in the [full reference](docs/REFERENCE.md#troubleshooting).

---

## More documentation

- [Full reference](docs/REFERENCE.md): features, memory, API, every config option, cellular (4G), headscale
- [Architecture](docs/ARCHITECTURE.md) · [Large tailnets](docs/LARGE_TAILNET.md) · [Testing guide](TESTING_GUIDE.md)

## Credits & license

MicroLink was created by **Cameron Malone** ([CamM2325](https://github.com/CamM2325)). This fork keeps his work and adds fixes on top. MIT License, see [LICENSE](LICENSE).
