# Haier BLE IR Remote — M5Atom Lite

A GitHub Pages-ready Progressive Web App (PWA) for controlling a Haier TV through an M5Atom Lite IR blaster over Bluetooth Low Energy.

## Confirmed TV commands

The current remote includes these commands confirmed during testing:

| Function | NEC | BLE command |
|---|---:|---|
| Channel + | `0x00` | `NEC:04:00` |
| Channel - | `0x01` | `NEC:04:01` |
| Volume + | `0x02` | `NEC:04:02` |
| Volume - | `0x03` | `NEC:04:03` |
| Power | `0x08` | `NEC:04:08` |
| Mute | `0x09` | `NEC:04:09` |
| Input / Source | `0x0B` | `NEC:04:0B` |

Navigation, Menu, Back, Exit, numeric keys, Info and Guide are intentionally shown as unassigned until their NEC codes are identified.

## BLE UUIDs

- Device: `M5-IR-Blaster`
- Service: `7b7e0001-1234-4567-89ab-123456789000`
- Command characteristic: `7b7e0002-1234-4567-89ab-123456789000`
- Status characteristic: `7b7e0003-1234-4567-89ab-123456789000`

## GitHub Pages

1. Create a GitHub repository.
2. Upload all files in this folder to the repository root.
3. In **Settings → Pages**, select **Deploy from a branch**, choose `main`, `/ (root)`.
4. Open the resulting `https://shahid-tk.github.io/HaierTV-ESP32-BLE-IR-Remote-Webapp` URL in a browser that supports Web Bluetooth.
5. Connect the M5Atom Lite using **Connect BLE**.
6. Use the remote.

## PWA / WebView

The site includes `manifest.webmanifest` and `sw.js`, so browsers that support PWA installation can install it as a standalone app.

Important: Web Bluetooth support depends on the browser/platform. GitHub Pages HTTPS is required. Android Chrome/Chromium-based browsers are the practical target. iOS Safari does not provide the same Web Bluetooth capability needed by this page.

## M5Atom firmware expectation

The existing firmware must expose:

- the service UUID above
- the command characteristic UUID above
- a write handler accepting strings such as `NEC:04:02`

The page does not require Wi-Fi; the browser communicates directly with the M5Atom over BLE.


## Arduino firmware

The repository includes `haier_ble_ir_blaster.ino`.

### Arduino IDE libraries

Install:

- **NimBLE-Arduino**

The firmware is intended for the M5Atom Lite / ESP32 and uses:

```cpp
#include <NimBLEDevice.h>
```

The IR LED is connected:

```text
GPIO25 (G25) → 220 Ω resistor → IR LED → GND
```

The firmware implements the NEC sender used by the web remote.

### Supported commands

The BLE command characteristic accepts strings such as:

```text
TEST
NEC:04:00
NEC:04:01
NEC:04:02
NEC:04:03
NEC:04:08
NEC:04:09
NEC:04:0B
```

The confirmed Haier mapping is:

```text
0x00  Channel +
0x01  Channel -
0x02  Volume +
0x03  Volume -
0x08  Power
0x09  Mute
0x0B  Input / Source
```
