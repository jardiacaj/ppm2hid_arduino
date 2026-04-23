#pragma once
#include <Arduino.h>

typedef struct {
    uint8_t expected_count;   // 0 = not yet locked
    uint8_t candidate_count;
    uint8_t stable_frames;
} ChannelLock;

// Returns true if the frame should be processed; false = discard.
// Direct port of cli.py CHANNEL_LOCK_FRAMES logic.
bool channel_lock_accept(ChannelLock *lock, uint8_t frame_ch_count);

void channel_lock_reset(ChannelLock *lock);
