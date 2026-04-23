# Contributing

Contributions are welcome. Please read this document before opening a pull request.

## Contributor License Agreement

By contributing to this project you agree to the [Contributor License Agreement](CLA.md). You confirm your agreement by adding your name and GitHub username to `CLA_SIGNATORIES.md` in your pull request.

## Reporting bugs

Open a [GitHub Issue](../../issues) with:
- Arduino board model (Leonardo / Pro Micro / other ATmega32U4 board)
- RC transmitter and receiver model
- Serial Monitor output (with `SERIAL_DEBUG 1`)
- What you expected vs. what happened

## Making changes

1. Fork the repository and create a branch from `main`:
   ```bash
   git checkout -b fix/describe-your-change
   ```

2. Make your changes. Keep each pull request focused on a single concern.

3. Test with a real RC receiver and verify with `jstest` or equivalent.

4. Add your name to `CLA_SIGNATORIES.md` if it is not already there.

5. Open a pull request against `main`. Describe what changed and why.

## Coding style

- C99/C++11, compatible with Arduino AVR toolchain (avr-g++ 7.x+)
- Indent with 4 spaces; no tabs
- Keep ISR code short and allocation-free — no `malloc`, no `String`, no `Serial` inside `ppm_isr()`
- Prefer `#if SERIAL_DEBUG` guards over runtime checks around debug prints
- New channel types or board support: add a section to `README.md`

## What belongs where

| File | Purpose |
|------|---------|
| `config.h` | Timing constants and feature flags only — no logic |
| `ppm_decoder.cpp` | ISR and PPM frame decoding — no HID calls |
| `channel_lock.cpp` | Channel count stability — no HID or PPM calls |
| `channel_map.cpp` | Channel→HID mapping, Joystick object — no PPM calls |
| `ppm2hid_arduino.ino` | Wiring the modules together — minimal logic |
