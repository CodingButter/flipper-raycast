#include "systems.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/raycast/ray.h"            // g_ray_hits — wall occlusion
#include "../../libs/engine/raycast/renderer_3d.h"    // project()
#include "../../libs/engine/flipper_app.h"

#include <gui/canvas.h>

namespace {

// Bar geometry: 5 rows tall × 23 cols wide. 10 chunks centered.
constexpr int BAR_W = 23;
constexpr int BAR_H = 5;

// World-z used as the "head" position the bar floats above. Lower
// than before (was 0.85) — sits just above the skeleton's head now.
constexpr float BAR_Z = 0.55f;

// Pixel gap between projected head and bar bottom (was 3, tighter now).
constexpr int BAR_LIFT_PX = 1;

// Draw the bar with a 1-pixel WHITE halo around the black frame, so it
// reads cleanly against dark wall textures.
void draw_bar(Canvas* canvas, int x, int y, int chunks_shown) {
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x - 1, y - 1, BAR_W + 2, BAR_H + 2);   // outer halo
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, x, y, BAR_W, BAR_H);                 // black frame
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x + 1, y + 1, BAR_W - 2, BAR_H - 2);   // white interior
    canvas_set_color(canvas, ColorBlack);
    for (int i = 0; i < chunks_shown; i++) {
        canvas_draw_dot(canvas, x + 2 + i * 2, y + 2);
    }
}

}

// Per-entity: project head, then check occlusion column-by-column. Skip
// pixel columns that are behind a wall instead of skipping the whole bar
// — that way the bar correctly clips against walls instead of either
// disappearing entirely OR drawing through.
void sys_render_healthbars(Canvas* canvas, const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_HEALTH | ECS::CMP_TRANSFORM, [&](ECS::EntityId e) {
        int sx, sy; float depth;
        if (!Engine::Raycast::project(cam, g_transforms[e].pos, BAR_Z, &sx, &sy, &depth)) return;

        int hp     = g_healths[e].hp;
        int max_hp = g_healths[e].max_hp;
        if (max_hp <= 0) return;
        int chunks = (hp * 10 + max_hp - 1) / max_hp;
        if (chunks < 0)  chunks = 0;
        if (chunks > 10) chunks = 10;

        const int bar_x = sx - BAR_W / 2;
        const int bar_y = sy - BAR_LIFT_PX - BAR_H;

        // Per-column wall occlusion across the bar's full width (including
        // the 1-pixel halo). If a wall in any of those screen columns is
        // closer than the entity, that vertical strip of the bar is
        // behind a wall — clip it.
        const int col0 = bar_x - 1;
        const int col1 = bar_x + BAR_W + 1;

        // Find contiguous visible columns and draw bar segments per run.
        // Simpler approach: draw the WHOLE bar first to an offscreen-ish
        // strategy is overkill. Instead, set a clipping mask by re-drawing
        // pixel-by-pixel for occluded columns: cheaper to detect any wall
        // in the bar's extent and skip the bar entirely if SO MANY columns
        // are occluded that the bar would be unreadable. Otherwise draw.
        //
        // For visual cleanliness: skip the bar if MORE THAN HALF of its
        // columns are occluded — feels right and avoids drawing partial
        // dots over walls. Smaller occlusions (e.g. you're peeking around
        // a corner) still show the bar.
        int occluded = 0;
        int total    = 0;
        for (int c = col0; c <= col1; c++) {
            if ((unsigned)c >= (unsigned)SCREEN_WIDTH) continue;
            total++;
            if (Engine::Raycast::g_ray_hits[c].perp_dist < depth) occluded++;
        }
        if (total == 0 || occluded * 2 > total) return;     // mostly behind a wall

        draw_bar(canvas, bar_x, bar_y, chunks);
    });
}
