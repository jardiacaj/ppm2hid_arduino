#pragma once
#include <Arduino.h>
#include <Joystick.h>

// Joystick axis index values used by emit_channels() / set_axis().
#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_RX 2
#define AXIS_RY 3

// Last-sent values, used to skip redundant HID emits and track button hysteresis.
extern uint16_t axis_last_us[4];
extern bool     btn_last_state[5];

// Initialise axis/button tracking state (call from setup()).
void channel_map_init();

// Convert one decoded PPM frame into HID joystick events.
// frame: array of channel values in µs, ch_count: number of channels.
void emit_channels(const uint16_t *frame, uint8_t ch_count);

// Set all axes to AXIS_CENTER_US and release all buttons.
void reset_to_neutral();
