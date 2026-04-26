// ppm2hid_arduino — RC receiver PPM input → USB HID joystick
//
// Target: Arduino Leonardo or Pro Micro (ATmega32U4)
// Requires: ArduinoJoystickLibrary by Matthew Heironimus
//
// See config.h to adjust pin, timing, and debug settings.
// See channel_map.cpp to modify the channel-to-axis/button mapping.

#include "config.h"
#include "ppm_decoder.h"
#include "channel_lock.h"
#include "channel_map.h"

// Joystick is defined in channel_map.cpp; declared extern here so the linker
// finds the single instance regardless of compilation order.
extern Joystick_ Joystick;

static ChannelLock  lock;
static uint32_t     last_valid_ms  = 0;
static bool         in_signal_loss = false;

// Built-in LED status: blink while link is unstable (no channel lock yet) or
// lost. 250 ms toggle → 500 ms full cycle.
static const uint16_t LED_BLINK_HALF_MS = 250;
static uint32_t       last_led_toggle_ms = 0;
static bool           led_state          = false;

void setup() {
#if SERIAL_DEBUG
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000) {}  // wait up to 3 s for USB serial
    Serial.println(F("ppm2hid_arduino starting"));
#endif

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    channel_map_init();   // initialise Joystick and axis/button state
    channel_lock_reset(&lock);
    ppm_decoder_init();   // attach ISR to PPM_INPUT_PIN

    last_valid_ms = millis();

#if SERIAL_DEBUG
    Serial.println(F("Waiting for PPM signal..."));
#endif
}

void loop() {
    if (isr_frame_ready) {
        // Atomically copy the ISR frame buffer.
        noInterrupts();
        uint8_t  ch_count = isr_channel_count;
        uint16_t frame[PPM_MAX_ISR_CHANNELS];
        for (uint8_t i = 0; i < ch_count; i++)
            frame[i] = isr_channels[i];
        isr_frame_ready = false;
        interrupts();

        if (!channel_lock_accept(&lock, ch_count))
            goto check_signal_loss;

        // First valid frame after signal was lost.
        if (in_signal_loss) {
            in_signal_loss = false;
#if SERIAL_DEBUG
            Serial.println(F("Signal restored"));
#endif
        }

        last_valid_ms = millis();
        emit_channels(frame, ch_count);

#if SERIAL_DEBUG
        Serial.print(F("ch:"));
        for (uint8_t i = 0; i < ch_count; i++) {
            Serial.print(' ');
            Serial.print(frame[i]);
        }
        Serial.println();
#endif
    }

check_signal_loss:
    if (!in_signal_loss && (millis() - last_valid_ms) > SIGNAL_LOSS_MS) {
        in_signal_loss = true;
        channel_lock_reset(&lock);
        reset_to_neutral();
#if SERIAL_DEBUG
        Serial.println(F("Signal lost — joystick reset to neutral"));
#endif
    }

    // Link unstable while signal lost or before the channel-count lock latches.
    bool link_unstable = in_signal_loss || (lock.expected_count == 0);
    if (link_unstable) {
        uint32_t now = millis();
        if (now - last_led_toggle_ms >= LED_BLINK_HALF_MS) {
            last_led_toggle_ms = now;
            led_state = !led_state;
            digitalWrite(LED_BUILTIN, led_state ? HIGH : LOW);
        }
    } else if (led_state) {
        led_state = false;
        digitalWrite(LED_BUILTIN, LOW);
    }
}
