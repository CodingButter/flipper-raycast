// ---------------------------------------------------------------------------
// FlipperApp — game-loop framework for Flipper Zero apps.
//
// Subclass it, override tick() and draw(), and call run() from your
// extern "C" entry point. The framework runs a fixed-FPS loop that drains
// input, calls your tick() (game logic), then asks the GUI to call draw().
//
// Input is *polled*, not callbacked. In tick(), ask the `input` member:
//   - input.is_pressed(key)   — currently held (continuous)
//   - input.was_pressed(key)  — became pressed THIS frame (edge-triggered)
//   - input.was_released(key) — became released THIS frame
// ---------------------------------------------------------------------------

#pragma once
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include "vector.h"
#include "flip2d.h"

// Flipper Zero's screen is a 128×64 monochrome LCD. All evaluated at
// compile time. Two forms of each constant:
//   - Scalar (int) for index / pixel-count math.
//   - Vec2 for vector math (subtracting positions, centering, etc.).
constexpr int SCREEN_WIDTH    = 128;
constexpr int SCREEN_HEIGHT   = 64;
constexpr int SCREEN_CENTER_X = SCREEN_WIDTH  / 2;
constexpr int SCREEN_CENTER_Y = SCREEN_HEIGHT / 2;
constexpr int SCREEN_MAX_X    = SCREEN_WIDTH  - 1;   // last valid pixel x
constexpr int SCREEN_MAX_Y    = SCREEN_HEIGHT - 1;   // last valid pixel y

constexpr Vec2 SCREEN_SIZE   {(float)SCREEN_WIDTH,    (float)SCREEN_HEIGHT};
constexpr Vec2 SCREEN_CENTER {(float)SCREEN_CENTER_X, (float)SCREEN_CENTER_Y};
constexpr Vec2 SCREEN_MAX    {(float)SCREEN_MAX_X,    (float)SCREEN_MAX_Y};

// Per-key input state, queried by your tick() each frame.
//
// Edge-triggered queries (was_pressed / was_released / was_double_tapped) are
// true for exactly the frame after the event happens — clear at end_frame.
//
// Double-tap note: was_double_tapped(k) does NOT suppress was_pressed(k) on
// the second tap — both are true. Disambiguate in your handler when needed:
//     if (input.was_double_tapped(Fire)) open_inventory();
//     else if (input.was_pressed(Fire))  shoot_bullet();
// Putting the double-tap check first means a fast tap-tap won't ALSO shoot.
class Input {
public:
    bool is_pressed(InputKey k)        const { return k < InputKeyMAX && held_[k]; }
    bool was_pressed(InputKey k)       const { return k < InputKeyMAX && pressed_[k]; }
    bool was_released(InputKey k)      const { return k < InputKeyMAX && released_[k]; }
    bool was_double_tapped(InputKey k) const { return k < InputKeyMAX && double_tapped_[k]; }

private:
    friend class FlipperApp;
    void apply(const InputEvent& e);          // called by framework as events arrive
    void end_frame();                         // called after each tick()
    void watchdog_release(uint32_t now_tick); // catch lost Release events

    // Max ms between two presses of the same key to count as a double-tap.
    static constexpr uint32_t DOUBLE_TAP_MS = 300;

    // Watchdog timeout — if a key claims to be held but we haven't received
    // ANY event for it in this many ms, assume its Release was dropped
    // (a known sporadic Flipper OS issue under load) and force-clear it.
    // 300 ms is comfortably longer than the OS's ~100 ms Repeat interval,
    // so genuine long holds keep refreshing the timestamp via Repeat events.
    static constexpr uint32_t WATCHDOG_TIMEOUT_MS = 300;

    bool     held_[InputKeyMAX]            = {};
    bool     pressed_[InputKeyMAX]         = {};
    bool     released_[InputKeyMAX]        = {};
    bool     double_tapped_[InputKeyMAX]   = {};
    bool     has_heartbeat_[InputKeyMAX]   = {};   // saw Long/Repeat → watchdog armed
    uint32_t last_press_tick_[InputKeyMAX] = {};   // for double-tap detection
    uint32_t last_event_tick_[InputKeyMAX] = {};   // for the watchdog
};

class FlipperApp {
public:
    // target_fps caps how often tick() runs (game logic + input polling).
    // draw_every_n_ticks: render every Nth tick — decouples the game loop
    //   rate from the visual rate. e.g., FlipperApp(30, 2) keeps input/logic
    //   at 30 Hz but only redraws at 15 Hz, halving GUI-thread CPU pressure.
    //   Default 1 = draw every tick (legacy behavior).
    explicit FlipperApp(int target_fps = 30, int draw_every_n_ticks = 1)
        : frame_ms_(1000 / target_fps), draw_every_(draw_every_n_ticks) {}
    virtual ~FlipperApp() = default;

    // Runs the game loop until tick() returns false. Returns loader status.
    int32_t run();

protected:
    // OPTIONAL override. Called once per frame on the app thread.
    //   dt: seconds elapsed since the previous tick (real measured time,
    //       not nominal frame time). Multiply your speeds by this so motion
    //       stays frame-rate-independent: position += velocity * dt.
    // Return true to keep running, false to exit.
    // Default: exit when Back is pressed. If you override, you must check
    // input.was_pressed(InputKeyBack) yourself if you still want that exit.
    virtual bool tick(float dt) { (void)dt; return !input.was_pressed(InputKeyBack); }

    // REQUIRED override. Called on the GUI thread when a repaint happens.
    // Must be fast and non-blocking. The Flip2D is stack-constructed by
    // the framework per call — no allocation, no cleanup needed.
    // NOTE: this runs on a different thread than tick(), so reading mutable
    // game state from here is technically racy — fine for small atomic
    // fields, bad for large structs you're mid-update.
    virtual void draw(Flip2D& c) = 0;

    Input input;

private:
    uint32_t frame_ms_;
    int      draw_every_;
    FuriMessageQueue* input_queue_ = nullptr;
    ViewPort* view_port_ = nullptr;
    Gui* gui_ = nullptr;

    static void draw_thunk(Canvas* canvas, void* ctx);
    static void input_thunk(InputEvent* event, void* ctx);
};
