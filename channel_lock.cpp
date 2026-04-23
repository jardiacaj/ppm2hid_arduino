#include "channel_lock.h"
#include "config.h"

bool channel_lock_accept(ChannelLock *lock, uint8_t frame_ch_count) {
    if (lock->expected_count == 0) {
        if (frame_ch_count == lock->candidate_count) {
            lock->stable_frames++;
            if (lock->stable_frames >= CHANNEL_LOCK_FRAMES) {
                lock->expected_count = frame_ch_count;
#if SERIAL_DEBUG
                Serial.print(F("Channel count locked: "));
                Serial.println(frame_ch_count);
#endif
            }
        } else {
            lock->candidate_count = frame_ch_count;
            lock->stable_frames   = 1;
        }
        return false;
    }

    if (frame_ch_count != lock->expected_count) {
#if SERIAL_DEBUG
        Serial.print(F("WARNING: channel count "));
        Serial.print(frame_ch_count);
        Serial.print(F(" != expected "));
        Serial.print(lock->expected_count);
        Serial.println(F(" — frame skipped"));
#endif
        return false;
    }

    return true;
}

void channel_lock_reset(ChannelLock *lock) {
    lock->expected_count  = 0;
    lock->candidate_count = 0;
    lock->stable_frames   = 0;
}
