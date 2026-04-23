#pragma once
#include <Arduino.h>

#define PPM_MAX_ISR_CHANNELS 10

// Written by ISR, read by main loop.
extern volatile uint16_t isr_channels[PPM_MAX_ISR_CHANNELS];
extern volatile uint8_t  isr_channel_count;
extern volatile bool     isr_frame_ready;

// Attach the ISR to PPM_INPUT_PIN. Call once from setup().
void ppm_decoder_init();

// The ISR itself — must be declared here for Arduino to accept it as an
// interrupt handler attached via attachInterrupt().
void ppm_isr();
