#include "systems.h"
#include "../state.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/flipper_app.h"
#include "../../libs/engine/haptic.h"

#include <gui/canvas.h>
#include <stdio.h>

// ─────────────────────────────────────────────────────────────────────
// Game loop: tick the level timer until the player reaches the goal
// cell. Distance² check is fine — no need to walk a path.
// ─────────────────────────────────────────────────────────────────────
void sys_level(const Engine::Raycast::Camera& cam, float dt) {
    if (Game::level_complete) return;

    Game::level_time += dt;

    Vec2 d = cam.position - Game::level_goal_pos;
    if (d.sqr_length() < Game::level_goal_radius_sq) {
        Game::level_complete      = true;
        Game::level_complete_time = Game::level_time;
        // Celebratory haptic — 3 quick pulses so it's distinct from
        // the per-shot/per-pickup feedback.
        Engine::Haptic::pulse(3, 60, 80);
    }
}

// ─────────────────────────────────────────────────────────────────────
// Stats overlay — full-screen panel when level_complete. Mirrors the
// inventory's visual treatment so the UX is consistent.
// ─────────────────────────────────────────────────────────────────────
void sys_render_stats(Canvas* canvas) {
    if (!Game::level_complete) return;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box  (canvas, 0, 0, SCREEN_WIDTH,     SCREEN_HEIGHT);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_frame(canvas, 0, 0, SCREEN_WIDTH,     SCREEN_HEIGHT);
    canvas_draw_frame(canvas, 1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, SCREEN_CENTER_X, 4,
                            AlignCenter, AlignTop, "LEVEL COMPLETE");
    canvas_draw_line(canvas, 14, 13, SCREEN_WIDTH - 14, 13);

    int total = (int)Game::level_complete_time;
    int mm = total / 60;
    int ss = total % 60;

    char buf[24];
    int y = 22;
    snprintf(buf, sizeof(buf), "TIME:    %02d:%02d", mm, ss);
    canvas_draw_str(canvas, 6, y, buf); y += 9;
    snprintf(buf, sizeof(buf), "KILLS:   %d", Game::enemies_killed);
    canvas_draw_str(canvas, 6, y, buf); y += 9;
    snprintf(buf, sizeof(buf), "COINS:   %d", Game::coins_collected);
    canvas_draw_str(canvas, 6, y, buf); y += 9;
    snprintf(buf, sizeof(buf), "HP LEFT: %d/%d",
             Game::player_health, Game::PLAYER_MAX_HEALTH);
    canvas_draw_str(canvas, 6, y, buf);

    canvas_draw_str(canvas, 4, 60, "OK restart  BACKx2 quit");
}
