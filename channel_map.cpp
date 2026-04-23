#include "channel_map.h"
#include "config.h"
#include <Joystick.h>

// Joystick object — defined here; extern-declared in the .ino.
// 5 buttons, 0 hat switches, X/Y/Rx/Ry axes only.
Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID,
    JOYSTICK_TYPE_JOYSTICK,
    NUM_BUTTONS,   // buttonCount
    0,             // hatSwitchCount
    true,          // includeXAxis
    true,          // includeYAxis
    false,         // includeZAxis
    true,          // includeRxAxis
    true,          // includeRyAxis
    false,         // includeRzAxis
    false,         // includeRudder
    false,         // includeThrottle
    false,         // includeAccelerator
    false,         // includeBrake
    false          // includeSteering
);

uint16_t axis_last_us[NUM_AXES];
bool     btn_last_state[NUM_BUTTONS];

// ── Channel map — Absima CR10P / Dumbo RC DDF-350 profile ─────────────────
//
// Type codes: 'A' = axis, 'B' = button, 'N' = n-pos (multi-button switch).
// Matches profiles/absima_cr10p.toml exactly.
//
// ch  type  axis / btn   invert  notes
//  1   A     AXIS_X        no    steering
//  2   A     AXIS_Y        yes   throttle
//  3   B     btn 0         —
//  4   B     btn 1         —
//  5   A     AXIS_RX       no
//  6   A     AXIS_RY       no
//  7   N     btn2/btn3     —     3-pos switch: thresholds 1300 / 1700 µs
//  8   B     btn 4         —

static void set_axis(uint8_t axis_idx, int32_t value_us) {
    switch (axis_idx) {
        case AXIS_X:  Joystick.setXAxis(value_us);  break;
        case AXIS_Y:  Joystick.setYAxis(value_us);  break;
        case AXIS_RX: Joystick.setRxAxis(value_us); break;
        case AXIS_RY: Joystick.setRyAxis(value_us); break;
    }
}

static void update_axis(uint8_t axis_idx, uint16_t raw_us, bool invert) {
    int32_t value = invert ? (AXIS_MIN_US + AXIS_MAX_US - (int32_t)raw_us) : (int32_t)raw_us;
    if (abs(value - (int32_t)axis_last_us[axis_idx]) >= AXIS_DEADBAND_US) {
        axis_last_us[axis_idx] = (uint16_t)value;
        set_axis(axis_idx, value);
    }
}

// Hysteresis follows ppm2hid uinput.py:
//   pressed state  → threshold is lowered by hysteresis (easier to release)
//   released state → threshold is raised by hysteresis (harder to press)
static void update_button(uint8_t btn_idx, uint16_t raw_us,
                          uint16_t threshold, uint16_t hysteresis) {
    int16_t hys     = btn_last_state[btn_idx] ? (int16_t)hysteresis : -(int16_t)hysteresis;
    bool    pressed = (int32_t)raw_us > ((int32_t)threshold - hys);
    if (pressed != btn_last_state[btn_idx]) {
        btn_last_state[btn_idx] = pressed;
        Joystick.setButton(btn_idx, pressed ? 1 : 0);
    }
}

void channel_map_init() {
    Joystick.setXAxisRange(AXIS_MIN_US,  AXIS_MAX_US);
    Joystick.setYAxisRange(AXIS_MIN_US,  AXIS_MAX_US);
    Joystick.setRxAxisRange(AXIS_MIN_US, AXIS_MAX_US);
    Joystick.setRyAxisRange(AXIS_MIN_US, AXIS_MAX_US);

    for (uint8_t i = 0; i < NUM_AXES;    i++) axis_last_us[i]    = AXIS_CENTER_US;
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) btn_last_state[i]  = false;

    // sendState = false: we call Joystick.sendState() manually once per frame.
    Joystick.begin(false);
}

void emit_channels(const uint16_t *frame, uint8_t ch_count) {
    // ch1 — steering (X axis, no invert)
    if (ch_count >= 1) update_axis(AXIS_X, frame[0], false);

    // ch2 — throttle (Y axis, inverted)
    if (ch_count >= 2) update_axis(AXIS_Y, frame[1], true);

    // ch3 — button 0
    if (ch_count >= 3)
        update_button(0, frame[2], BUTTON_THRESHOLD_US, BUTTON_HYSTERESIS_US);

    // ch4 — button 1
    if (ch_count >= 4)
        update_button(1, frame[3], BUTTON_THRESHOLD_US, BUTTON_HYSTERESIS_US);

    // ch5 — Rx axis
    if (ch_count >= 5) update_axis(AXIS_RX, frame[4], false);

    // ch6 — Ry axis
    if (ch_count >= 6) update_axis(AXIS_RY, frame[5], false);

    // ch7 — 3-position switch → button 2 (thresh 1300) and button 3 (thresh 1700)
    if (ch_count >= 7) {
        update_button(2, frame[6], 1300, BUTTON_HYSTERESIS_US);
        update_button(3, frame[6], 1700, BUTTON_HYSTERESIS_US);
    }

    // ch8 — button 4
    if (ch_count >= 8)
        update_button(4, frame[7], BUTTON_THRESHOLD_US, BUTTON_HYSTERESIS_US);

    Joystick.sendState();
}

void reset_to_neutral() {
    for (uint8_t i = 0; i < NUM_AXES; i++) {
        axis_last_us[i] = AXIS_CENTER_US;
        set_axis(i, AXIS_CENTER_US);
    }
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        btn_last_state[i] = false;
        Joystick.setButton(i, 0);
    }
    Joystick.sendState();
}
