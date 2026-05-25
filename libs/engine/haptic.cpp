#include "haptic.h"

#include <stdint.h>
#include <furi_hal_vibro.h>

namespace Engine { namespace Haptic {

namespace {

constexpr int MAX_SEGMENTS = 16;

struct Segment {
    uint16_t duration_ms;
    bool     motor_on;
};

Segment s_queue[MAX_SEGMENTS];
int     s_count        = 0;        // segments in queue
int     s_index        = 0;        // current segment
float   s_remaining_s  = 0.0f;     // seconds left in current segment
bool    s_motor_state  = false;    // last command sent to the motor

void set_motor(bool on) {
    if (on == s_motor_state) return;
    furi_hal_vibro_on(on);
    s_motor_state = on;
}

void start_segment(int idx) {
    s_index = idx;
    if (idx >= s_count) {
        set_motor(false);
        return;
    }
    s_remaining_s = (float)s_queue[idx].duration_ms / 1000.0f;
    set_motor(s_queue[idx].motor_on);
}

void play(const Segment* segs, int n) {
    if (n <= 0) return;
    if (n > MAX_SEGMENTS) n = MAX_SEGMENTS;
    for (int i = 0; i < n; i++) s_queue[i] = segs[i];
    s_count = n;
    start_segment(0);
}

} // anonymous

void blast(int duration_ms) {
    Segment seq[1] = { { (uint16_t)duration_ms, true } };
    play(seq, 1);
}

void pulse(int count, int on_ms, int gap_ms) {
    if (count <= 0) return;
    Segment seq[MAX_SEGMENTS];
    int n = 0;
    for (int p = 0; p < count && n < MAX_SEGMENTS; p++) {
        seq[n++] = { (uint16_t)on_ms, true };
        if (p < count - 1 && n < MAX_SEGMENTS) {
            seq[n++] = { (uint16_t)gap_ms, false };
        }
    }
    play(seq, n);
}

void tick(float dt) {
    if (s_index >= s_count) {
        set_motor(false);
        return;
    }
    s_remaining_s -= dt;
    if (s_remaining_s <= 0.0f) start_segment(s_index + 1);
}

void stop() {
    s_count = 0;
    s_index = 0;
    set_motor(false);
}

}} // namespace Engine::Haptic
