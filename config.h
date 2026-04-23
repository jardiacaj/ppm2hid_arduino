#pragma once

// ── PPM signal input ───────────────────────────────────────────────────────
// INT0 (pin 2) or INT1 (pin 3) on Arduino Leonardo / Pro Micro (ATmega32U4).
#define PPM_INPUT_PIN        2
// Set to 1 if your receiver outputs LOW-active (inverted) PPM.
#define PPM_INVERTED         0

// ── PPM timing (microseconds) ── from ppm2hid Absima CR10P defaults ───────
#define SYNC_MIN_US          3000UL   // gap >= this → frame sync
#define SYNC_MAX_US          50000UL  // gap >  this → signal loss / noise
#define CHANNEL_MIN_US       500UL    // shortest valid channel pulse
#define CHANNEL_MAX_US       2100UL   // longest valid channel pulse
#define AXIS_MIN_US          1100
#define AXIS_MAX_US          1900
#define AXIS_CENTER_US       1500

// ── HID output ────────────────────────────────────────────────────────────
#define NUM_CHANNELS         8   // PPM channels per frame (Absima CR10P sends 8)
#define NUM_AXES             4   // X, Y, Rx, Ry
#define NUM_BUTTONS          5   // ch3, ch4, ch7a, ch7b, ch8

// ── Axis deadband ─────────────────────────────────────────────────────────
// Suppress HID reports when a stick hasn't moved beyond this many µs.
#define AXIS_DEADBAND_US     42

// ── Button thresholds ─────────────────────────────────────────────────────
#define BUTTON_THRESHOLD_US  1500
#define BUTTON_HYSTERESIS_US 21

// ── Channel count stability lock ──────────────────────────────────────────
// Require this many consecutive frames with the same channel count before
// accepting that count as valid (matches ppm2hid CHANNEL_LOCK_FRAMES = 5).
#define CHANNEL_LOCK_FRAMES  5

// ── Signal loss ───────────────────────────────────────────────────────────
// Reset joystick to neutral after this many ms without a valid frame.
#define SIGNAL_LOSS_MS       500

// ── Serial debug ──────────────────────────────────────────────────────────
// Set to 0 before flashing a production device to reduce USB interrupt load.
#define SERIAL_DEBUG         1
#define SERIAL_BAUD          115200
