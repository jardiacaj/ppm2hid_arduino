# ppm2hid_arduino

Arduino firmware that reads a PPM signal from an RC receiver and presents the board as a USB HID joystick — no PC software, no audio interface, no Linux required.

Works on **Arduino Leonardo** and **Arduino Pro Micro** (ATmega32U4), the only common Arduino-class boards with native USB HID support.

Default channel map matches the **Absima CR10P / Dumbo RC DDF-350** transmitter. Mapping is easy to change in `channel_map.cpp`.

Related project: [ppm2hid](https://github.com/jardiacaj/ppm2hid) — the Python/Linux equivalent that reads PPM via an audio Line In jack.

---

## Hardware requirements

- Arduino Leonardo or Pro Micro (ATmega32U4)
- RC receiver with a PPM output (also called "sum signal" or "CPPM")
- 3-wire connection: PPM signal, +5 V (or 3.3 V), GND

## Wiring

```
RC Receiver                  Arduino Leonardo / Pro Micro
─────────────────────────────────────────────────────────
PPM OUT ─────────────────────── Pin 2  (INT0)
GND ─────────────────────────── GND
VCC ─────────────────────────── 5V
```

Pin 2 (INT0) is the default. Pin 3 (INT1) also works — change `PPM_INPUT_PIN` in `config.h`.

**Powering the receiver from the Arduino 5V pin:** The 5V pin is the USB bus supply through a polyfuse (500 mA total budget). The board itself draws ~50–100 mA, leaving ~400 mA for the receiver — sufficient for any standard receiver not driving servos directly. Do not use a GPIO pin for power; GPIO pins are limited to 40 mA and will be damaged by receiver current draw. If the receiver also powers servos, use a separate BEC or battery pack and share only GND with the Arduino.

**3.3 V receivers:** The ATmega32U4 input threshold is ~1.5 V; a 3.3 V signal is detected reliably without level shifting.

**Noise reduction (optional):** Add a 100 nF capacitor between Pin 2 and GND, close to the connector.

**Inverted PPM:** Some receivers output LOW-active PPM. Set `PPM_INVERTED 1` in `config.h`.

---

## Software requirements

1. **Arduino IDE** 1.8+ or 2.x
2. **Board support:** Arduino AVR Boards (built-in) — select *Arduino Leonardo* or *SparkFun Pro Micro*
3. **Library:** [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary) by Matthew Heironimus
   - Install via Arduino IDE → **Tools → Manage Libraries** → search `Joystick by Matthew Heironimus`

---

## Installation

1. Clone or download this repository.
2. Open `ppm2hid_arduino/ppm2hid_arduino.ino` in the Arduino IDE.
3. Install ArduinoJoystickLibrary (see above).
4. Select the correct board and port under **Tools**.
5. Click **Upload**.

---

## Configuration (`config.h`)

| Define | Default | Description |
|--------|---------|-------------|
| `PPM_INPUT_PIN` | `2` | Interrupt-capable pin: 2 (INT0) or 3 (INT1) |
| `PPM_INVERTED` | `0` | `1` = LOW-active (inverted) PPM signal |
| `SYNC_MIN_US` | `3000` | Minimum sync gap in µs |
| `SYNC_MAX_US` | `50000` | Maximum sync gap in µs (longer = signal loss) |
| `CHANNEL_MIN_US` | `500` | Shortest valid channel pulse |
| `CHANNEL_MAX_US` | `2100` | Longest valid channel pulse |
| `AXIS_MIN_US` | `1100` | Axis minimum value (µs) |
| `AXIS_MAX_US` | `1900` | Axis maximum value (µs) |
| `AXIS_CENTER_US` | `1500` | Axis neutral/centre value |
| `AXIS_DEADBAND_US` | `42` | Suppress axis updates within ±N µs |
| `BUTTON_THRESHOLD_US` | `1500` | PPM value above which a button is pressed |
| `BUTTON_HYSTERESIS_US` | `21` | Hysteresis around button threshold |
| `CHANNEL_LOCK_FRAMES` | `5` | Stable frames required before accepting channel count |
| `SIGNAL_LOSS_MS` | `500` | ms without a valid frame before joystick resets to neutral |
| `SERIAL_DEBUG` | `1` | Print decoded frames to Serial Monitor (set `0` for production) |

---

## Default channel map (Absima CR10P)

| Channel | Type | Joystick output | Inverted |
|---------|------|-----------------|----------|
| 1 | Axis | X (steering) | No |
| 2 | Axis | Y (throttle) | Yes |
| 3 | Button | Button 0 | — |
| 4 | Button | Button 1 | — |
| 5 | Axis | Rx | No |
| 6 | Axis | Ry | No |
| 7 | 3-pos switch | Button 2 (>1300 µs) + Button 3 (>1700 µs) | — |
| 8 | Button | Button 4 | — |

To change the mapping, edit `channel_map.cpp` → `emit_channels()`.

---

## Verification

**Serial Monitor:** With `SERIAL_DEBUG 1`, open the Serial Monitor at 115200 baud. You should see:

```
ppm2hid_arduino starting
Waiting for PPM signal...
Channel count locked: 8
ch: 1500 1500 1500 1500 1500 1500 1500 1500
```

Moving sticks should change the corresponding values.

**Linux:**

```bash
# List joystick devices
ls /dev/input/js*

# Read raw joystick events (move sticks to see axis/button output)
jstest /dev/input/js0

# Full evdev event stream
evtest /dev/input/event<N>
```

**Windows / macOS:** The device appears in **Game Controllers** / **Joystick** system settings.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| "Waiting for PPM signal..." never advances | Wrong pin, no PPM signal, or inverted PPM | Check wiring; try `PPM_INVERTED 1` |
| Channel count never locks | Noisy signal or wrong timing | Add bypass cap; check `CHANNEL_MIN_US`/`CHANNEL_MAX_US` in `config.h` |
| Axes jitter at neutral | Excessive noise | Increase `AXIS_DEADBAND_US` |
| Throttle axis reversed | Inversion not applied | Set `invert = true` in `update_axis()` call for ch2 in `channel_map.cpp` (already set by default) |
| Device not recognised as joystick | Wrong board | Must be Leonardo or Pro Micro (ATmega32U4) |

---

## License

MIT — see [LICENSE](LICENSE).
