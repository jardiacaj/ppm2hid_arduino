#include "ppm_decoder.h"
#include "config.h"

volatile uint16_t isr_channels[PPM_MAX_ISR_CHANNELS];
volatile uint8_t  isr_channel_count = 0;
volatile bool     isr_frame_ready   = false;

// ISR-private state (not accessed from main loop).
static volatile uint32_t _last_edge_us  = 0;
static volatile uint8_t  _ch_idx        = 0;
static volatile bool     _synced        = false;
static uint16_t          _frame_buf[PPM_MAX_ISR_CHANNELS];

// The ISR triggers on FALLING edge (or RISING for inverted PPM).
// Each interval between two consecutive same-polarity edges equals the
// previous channel's HIGH + LOW duration — the total channel value in µs.
// This matches the ppm2hid two-phase pending_high + low_length measurement.
void ppm_isr() {
    uint32_t now         = micros();
    uint32_t interval_us = now - _last_edge_us;
    _last_edge_us        = now;

    if (interval_us >= SYNC_MIN_US && interval_us <= SYNC_MAX_US) {
        // Sync gap: commit accumulated frame if it has at least 2 channels.
        if (_synced && _ch_idx >= 2) {
            for (uint8_t i = 0; i < _ch_idx; i++)
                isr_channels[i] = _frame_buf[i];
            isr_channel_count = _ch_idx;
            isr_frame_ready   = true;
        }
        _ch_idx  = 0;
        _synced  = true;

    } else if (interval_us > SYNC_MAX_US) {
        // Implausibly long gap — signal loss or noise; lose sync.
        _synced = false;
        _ch_idx = 0;

    } else if (_synced && interval_us >= CHANNEL_MIN_US && interval_us <= CHANNEL_MAX_US) {
        if (_ch_idx < PPM_MAX_ISR_CHANNELS) {
            uint16_t v = (uint16_t)interval_us;
            if (v < AXIS_MIN_US) v = AXIS_MIN_US;
            if (v > AXIS_MAX_US) v = AXIS_MAX_US;
            _frame_buf[_ch_idx++] = v;
        }
    } else {
        // Out-of-range pulse while synced: discard and lose sync.
        _synced = false;
        _ch_idx = 0;
    }
}

void ppm_decoder_init() {
    pinMode(PPM_INPUT_PIN, INPUT);
    attachInterrupt(
        digitalPinToInterrupt(PPM_INPUT_PIN),
        ppm_isr,
        PPM_INVERTED ? RISING : FALLING
    );
}
