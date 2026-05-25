#include "flipper_app.h"
#include "haptic.h"

void Input::apply(const InputEvent& e) {
    if (e.key >= InputKeyMAX) return;
    const uint32_t now = furi_get_tick();

    switch (e.type) {
        case InputTypePress: {
            held_[e.key]            = true;
            pressed_[e.key]         = true;
            has_heartbeat_[e.key]   = false;     // fresh press — disarm watchdog
            last_event_tick_[e.key] = now;
            // Double-tap detection.
            const uint32_t last = last_press_tick_[e.key];
            if (last != 0 && (now - last) <= DOUBLE_TAP_MS) {
                double_tapped_[e.key]   = true;
                last_press_tick_[e.key] = 0;
            } else {
                last_press_tick_[e.key] = now;
            }
            break;
        }
        case InputTypeRelease:
            held_[e.key]          = false;
            released_[e.key]      = true;
            has_heartbeat_[e.key] = false;
            // Don't refresh last_event_tick — we want any stale Repeats
            // arriving after this to be ignored (handled below).
            break;
        case InputTypeShort:
            if (held_[e.key]) {
                held_[e.key]     = false;
                released_[e.key] = true;
            }
            has_heartbeat_[e.key] = false;
            break;
        case InputTypeLong:
        case InputTypeRepeat:
            // Heartbeat ONLY counts while we already think the key is
            // held. Otherwise it's a stale event from before a Release
            // we already processed — ignoring it prevents the key from
            // "sticking back on" after release.
            if (held_[e.key]) {
                has_heartbeat_[e.key]   = true;
                last_event_tick_[e.key] = now;
            }
            break;
        default:
            break;
    }
}

// Watchdog runs only on keys that have proven the OS is sending
// heartbeats (Long/Repeat). For keys held briefly (< OS long-press
// threshold) we never enter the watched state — those rely on the
// Short fallback above for dropped-Release recovery.
void Input::watchdog_release(uint32_t now_tick) {
    for (int k = 0; k < InputKeyMAX; k++) {
        if (!held_[k]) continue;
        if (!has_heartbeat_[k]) continue;            // not armed yet
        if ((uint32_t)(now_tick - last_event_tick_[k]) > WATCHDOG_TIMEOUT_MS) {
            held_[k]          = false;
            released_[k]      = true;
            has_heartbeat_[k] = false;
        }
    }
}

void Input::end_frame() {
    for (int i = 0; i < InputKeyMAX; i++) {
        pressed_[i]       = false;
        released_[i]      = false;
        double_tapped_[i] = false;
    }
}

void FlipperApp::draw_thunk(Canvas* canvas, void* ctx) {
    auto* self = static_cast<FlipperApp*>(ctx);
    Flip2D c(canvas);   // stack-allocated, gone when this function returns
    self->draw(c);
}

void FlipperApp::input_thunk(InputEvent* event, void* ctx) {
    // All event types reach the queue:
    //   - Press / Release: primary state transitions
    //   - Short:           defensive backstop for missed Release on quick taps
    //   - Long / Repeat:   heartbeat the watchdog uses to detect a missed
    //                      Release on a long hold
    // Repeat events are ~10 Hz per held key; with a 30 Hz tick we drain
    // every 33 ms so the 64-slot queue can't back up under any realistic
    // input pattern (worst case 6 keys × 10 Hz × 33 ms = 2 events/drain).
    auto* self = static_cast<FlipperApp*>(ctx);
    furi_message_queue_put(self->input_queue_, event, FuriWaitForever);
}

int32_t FlipperApp::run() {
    // 64 slots — sized for Press+Release+Long+Repeat events arriving from
    // all 6 buttons during sustained input. Repeats are ~10 Hz per held
    // key, so this is comfortably oversized for our 30 Hz drain rate.
    input_queue_ = furi_message_queue_alloc(64, sizeof(InputEvent));

    view_port_ = view_port_alloc();
    view_port_draw_callback_set(view_port_, &FlipperApp::draw_thunk, this);
    view_port_input_callback_set(view_port_, &FlipperApp::input_thunk, this);

    gui_ = (Gui*)furi_record_open(RECORD_GUI);
    gui_add_view_port(gui_, view_port_, GuiLayerFullscreen);

    uint32_t last_frame     = furi_get_tick();
    uint32_t last_tick_time = furi_get_tick();
    int      draw_counter   = 0;
    while (true) {
        // 1. Drain every pending input event (non-blocking) into per-key state.
        InputEvent event;
        while (furi_message_queue_get(input_queue_, &event, 0) == FuriStatusOk) {
            input.apply(event);
        }

        // 1b. Watchdog — detects keys claiming "held" with no recent events
        //     (i.e. the OS dropped their Release). Force-releases them so
        //     the player doesn't get stranded mid-movement.
        const uint32_t now = furi_get_tick();
        input.watchdog_release(now);

        // 2. App's per-frame logic. Compute real elapsed time since the
        //    previous tick start so motion stays consistent even if a tick
        //    runs long. Convert ms to seconds for the natural "px/sec" feel.
        float dt    = (now - last_tick_time) / 1000.0f;
        last_tick_time = now;
        if (!tick(dt)) break;

        // 3. Edge flags (was_pressed / was_released) have now been seen by
        //    tick() — clear them so they're true for exactly one frame.
        input.end_frame();
        Engine::Haptic::tick(dt);   // advance any in-flight vibration pattern

        // 4. Ask the GUI service to call our draw_thunk → draw() on its thread.
        //    Only every Nth tick so the GUI thread doesn't monopolize the CPU
        //    when draws are heavy. Input/logic still run at the full tick rate.
        if (++draw_counter >= draw_every_) {
            draw_counter = 0;
            view_port_update(view_port_);
        }

        // 5. Frame pacing: sleep the remainder of this frame's budget.
        //    If tick() ran long, we skip the sleep and rebase to "now".
        uint32_t elapsed = furi_get_tick() - last_frame;
        if (elapsed < frame_ms_) {
            furi_delay_ms(frame_ms_ - elapsed);
        }
        last_frame = furi_get_tick();
    }

    Engine::Haptic::stop();   // don't leave the motor running
    view_port_enabled_set(view_port_, false);
    gui_remove_view_port(gui_, view_port_);
    view_port_free(view_port_);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(input_queue_);

    return 0;
}
