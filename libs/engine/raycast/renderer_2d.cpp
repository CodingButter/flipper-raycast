#include "renderer_2d.h"
#include "world_api.h"
#include "ray.h"
#include "../flipper_app.h"   // SCREEN_*
#include <math.h>

namespace Engine { namespace Raycast {

// Half-screen, expressed in grid units. Computed from the game-provided
// CELL_SIZE so changes to it at runtime stay consistent.
static inline Vec2 screen_center_grid() {
    return { (float)SCREEN_CENTER.x / (float)CELL_SIZE,
             (float)SCREEN_CENTER.y / (float)CELL_SIZE };
}

Vec2 grid_to_screen(const Vec2& grid, const Camera& cam) {
    Vec2 offset = grid - cam.position + screen_center_grid();
    return { offset.x * CELL_SIZE, offset.y * CELL_SIZE };
}

static void draw_wall_box(Bitmap1& b, const Vec2& cell, const Camera& camera) {
    Vec2 top_left  = grid_to_screen(cell,              camera);
    Vec2 bot_right = grid_to_screen(cell + Vec2{1, 1}, camera);
    int  x = (int)top_left.x;
    int  y = (int)top_left.y;
    int  w = (int)bot_right.x - x;
    int  h = (int)bot_right.y - y;
    b.fill_box(x, y, w, h);
}

void draw_walls(Bitmap1& b, const Camera& camera) {
    const Vec2 scg = screen_center_grid();
    int x_min = (int)(camera.position.x - scg.x);
    int x_max = (int)(camera.position.x + scg.x) + 1;
    int y_min = (int)(camera.position.y - scg.y);
    int y_max = (int)(camera.position.y + scg.y) + 1;
    if (x_min < 0)          x_min = 0;
    if (x_max > WORLD_COLS) x_max = WORLD_COLS;
    if (y_min < 0)          y_min = 0;
    if (y_max > WORLD_ROWS) y_max = WORLD_ROWS;
    for (int y = y_min; y < y_max; y++) {
        for (int x = x_min; x < x_max; x++) {
            if (cell_at(x, y)) draw_wall_box(b, Vec2{(float)x, (float)y}, camera);
        }
    }
}

void draw_grid(Bitmap1& b, const Camera& camera) {
    Vec2 grid_origin = grid_to_screen(Vec2{0, 0}, camera);
    Vec2 grid_end    = grid_to_screen(Vec2{(float)WORLD_COLS, (float)WORLD_ROWS}, camera);
    int gl = (grid_origin.x < 0)             ? 0             : (int)grid_origin.x;
    int gr = (grid_end.x    > SCREEN_WIDTH)  ? SCREEN_WIDTH  : (int)grid_end.x;
    int gt = (grid_origin.y < 0)             ? 0             : (int)grid_origin.y;
    int gb = (grid_end.y    > SCREEN_HEIGHT) ? SCREEN_HEIGHT : (int)grid_end.y;
    for (int col = 0; col <= WORLD_COLS; col++) {
        int sx = (int)grid_to_screen(Vec2{(float)col, 0}, camera).x;
        if (sx < 0 || sx > SCREEN_WIDTH) continue;
        b.draw_line(sx, gt, sx, gb);
    }
    for (int row = 0; row <= WORLD_ROWS; row++) {
        int sy = (int)grid_to_screen(Vec2{0, (float)row}, camera).y;
        if (sy < 0 || sy > SCREEN_HEIGHT) continue;
        b.draw_line(gl, sy, gr, sy);
    }
}

// Draws the actual view frustum as a trapezoid: near-plane to far-plane.
void draw_fov(Bitmap1& b, const Camera& camera) {
    Vec2 pls = grid_to_screen(camera.pl, camera);
    Vec2 prs = grid_to_screen(camera.pr, camera);
    Vec2 fls = grid_to_screen(camera.fl, camera);
    Vec2 frs = grid_to_screen(camera.fr, camera);
    b.draw_line((int)pls.x, (int)pls.y, (int)fls.x, (int)fls.y);
    b.draw_line((int)fls.x, (int)fls.y, (int)frs.x, (int)frs.y);
    b.draw_line((int)frs.x, (int)frs.y, (int)prs.x, (int)prs.y);
    b.draw_line((int)prs.x, (int)prs.y, (int)pls.x, (int)pls.y);
}

// 3×3 square centered on the camera position — middle pixel IS the position.
void draw_camera_marker(Bitmap1& b, const Camera& camera) {
    Vec2 sp = grid_to_screen(camera.position, camera);
    b.fill_box((int)sp.x - 1, (int)sp.y - 1, 3, 3);
}

void draw_ray_hits(Bitmap1& b, const Camera& camera) {
    const Vec2 view_origin = camera.position - screen_center_grid();
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        int hx = (int)((g_ray_hits[i].point.x - view_origin.x) * CELL_SIZE);
        int hy = (int)((g_ray_hits[i].point.y - view_origin.y) * CELL_SIZE);
        b.set_pixel(hx, hy);
    }
}

}} // namespace Engine::Raycast
