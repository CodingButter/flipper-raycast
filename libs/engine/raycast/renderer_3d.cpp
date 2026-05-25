#include "renderer_3d.h"
#include "ray.h"
#include "world_api.h"        // WALL_TEXTURES / WALL_TEXTURE_COUNT
#include "../flipper_app.h"   // SCREEN_WIDTH / SCREEN_HEIGHT
#include <math.h>

namespace Engine { namespace Raycast {

// Atlas pixels are packed 2-bits-per-pixel (4 pixels per byte, little-
// endian inside the byte). Decode one pixel's 0/1/2 value:
//   byte_index   = pixel_index >> 2
//   bit_position = (pixel_index & 3) * 2
// Inlined so per-pixel cost in the wall/sprite hot loops stays low.
static inline uint8_t atlas_px(const uint8_t* data, int idx) {
    return (data[idx >> 2] >> ((idx & 3) << 1)) & 0x3;
}

void draw_wall_strips(Bitmap1& b) {
    for (int col = 0; col < SCREEN_WIDTH; col++) {
        const RayHit& h = g_ray_hits[col];
        if (h.perp_dist < 0.01f) continue;
        if (h.texture_id <= 0) continue;
        if (h.texture_id >= WALL_TEXTURE_COUNT) continue;
        const Engine::Texture* tex = WALL_TEXTURES[h.texture_id];
        if (!tex) continue;

        const int TW = tex->w;
        const int TH = tex->h;

        // Full intended strip height; unclamped so the V stepper maps
        // correctly when the strip overflows the screen.
        int strip_h = (int)(SCREEN_HEIGHT / h.perp_dist);
        if (strip_h <= 0) continue;
        int unclamped_top = (SCREEN_HEIGHT - strip_h) / 2;

        int y_top = unclamped_top;
        int y_bot = y_top + strip_h - 1;
        if (y_top < 0)              y_top = 0;
        if (y_bot >= SCREEN_HEIGHT) y_bot = SCREEN_HEIGHT - 1;

        int tex_x = (int)(h.u * TW);
        if (tex_x >= TW) tex_x = TW - 1;
        if (tex_x < 0)   tex_x = 0;

        int v_step = (TH << 16) / strip_h;
        int v_pos  = (y_top - unclamped_top) * v_step;

        for (int y = y_top; y <= y_bot; y++) {
            int tex_y = v_pos >> 16;
            if (tex_y >= TH) tex_y = TH - 1;
            uint8_t p = atlas_px(tex->data, tex_y * TW + tex_x);
            if      (p == 1) b.set_pixel(col, y);
            else if (p == 2) b.clear_pixel(col, y);
            v_pos += v_step;
        }
    }
}

bool project(const Camera& cam, Vec2 world_xy, float z,
             int* out_sx, int* out_sy, float* out_depth) {
    const float cos_r        = cosf(cam.rotation);
    const float sin_r        = sinf(cam.rotation);
    const float tan_half_fov = tanf(cam.fov * 0.5f);

    Vec2  delta   = world_xy - cam.position;
    float depth   = delta.x * cos_r + delta.y * sin_r;
    if (depth < 0.01f) return false;
    float lateral = -delta.x * sin_r + delta.y * cos_r;

    float norm_x = (lateral / depth) / tan_half_fov;
    *out_sx     = (int)((SCREEN_WIDTH / 2) * (1.0f + norm_x));
    *out_sy     = (SCREEN_HEIGHT / 2) - (int)(z * SCREEN_HEIGHT / depth);
    *out_depth  = depth;
    return true;
}

void draw_billboard(Bitmap1& b, const Camera& cam,
                    Vec2 world_pos, float z,
                    const Engine::SpriteSheet* sheet, int frame, float scale,
                    bool flip_horizontal, bool color_invert) {
    if (!sheet) return;

    const float cos_r        = cosf(cam.rotation);
    const float sin_r        = sinf(cam.rotation);
    const float tan_half_fov = tanf(cam.fov * 0.5f);

    Vec2  delta   = world_pos - cam.position;
    float depth   = delta.x * cos_r + delta.y * sin_r;
    if (depth < 0.01f) return;
    float lateral = -delta.x * sin_r + delta.y * cos_r;

    // Same vertical projection factor as walls. A 1-world-unit-tall sprite
    // at depth `d` is SCREEN_HEIGHT/d pixels tall; multiplied by scale.
    int height_px = (int)(scale * SCREEN_HEIGHT / depth);
    if (height_px <= 0) return;
    // Width follows from the source frame's aspect, so tall/narrow sources
    // (skeletons, etc.) render tall/narrow instead of being squashed to a
    // square box.
    int width_px = height_px * sheet->frame_w / sheet->frame_h;
    if (width_px <= 0) return;

    // z above the ground shifts the sprite up on screen by z * vertical
    // projection (same SCREEN_HEIGHT/depth factor as the height).
    int z_offset_px = (int)(z * SCREEN_HEIGHT / depth);

    float norm_x      = (lateral / depth) / tan_half_fov;
    int   center_x    = (int)((SCREEN_WIDTH / 2) * (1.0f + norm_x));
    int   sprite_left = center_x - width_px  / 2;
    int   sprite_top  = (SCREEN_HEIGHT - height_px) / 2 - z_offset_px;

    const int u_step = (sheet->frame_w << 16) / width_px;
    const int v_step = (sheet->frame_h << 16) / height_px;

    // Packed atlas: each frame occupies (frame_w*frame_h)/4 bytes since
    // pixels are 2 bpp. Frame sizes are chosen to be multiples of 4
    // pixels so this division is exact.
    const uint8_t* frame_data =
        sheet->data + (frame * sheet->frame_w * sheet->frame_h) / 4;

    int col_start = sprite_left;
    int col_end   = sprite_left + width_px;
    if (col_start < 0)            col_start = 0;
    if (col_end   > SCREEN_WIDTH) col_end   = SCREEN_WIDTH;

    int row_start = sprite_top;
    int row_end   = sprite_top + height_px;
    if (row_start < 0)             row_start = 0;
    if (row_end   > SCREEN_HEIGHT) row_end   = SCREEN_HEIGHT;

    for (int col = col_start; col < col_end; col++) {
        if (g_ray_hits[col].perp_dist < depth) continue;   // occluded by wall

        int tex_u = ((col - sprite_left) * u_step) >> 16;
        if (tex_u >= sheet->frame_w) tex_u = sheet->frame_w - 1;
        if (tex_u < 0)               tex_u = 0;
        if (flip_horizontal) tex_u = sheet->frame_w - 1 - tex_u;

        int v_pos = (row_start - sprite_top) * v_step;
        for (int row = row_start; row < row_end; row++) {
            int tex_v = v_pos >> 16;
            if (tex_v >= sheet->frame_h) tex_v = sheet->frame_h - 1;

            const int row_stride = sheet->frame_w;
            uint8_t p = atlas_px(frame_data, tex_v * row_stride + tex_u);
            if (color_invert) {
                if      (p == 1) p = 2;
                else if (p == 2) p = 1;
            }
            if (p == 1) {
                b.set_pixel(col, row);
            } else if (p == 2) {
                b.clear_pixel(col, row);
            } else {
                // Transparent — draw 1-pixel WHITE outline if any
                // 4-neighbor pixel in source is black.
                bool border = false;
                if      (tex_u > 0                    && atlas_px(frame_data, tex_v * row_stride + (tex_u - 1)) == 1) border = true;
                else if (tex_u < sheet->frame_w - 1   && atlas_px(frame_data, tex_v * row_stride + (tex_u + 1)) == 1) border = true;
                else if (tex_v > 0                    && atlas_px(frame_data, (tex_v - 1) * row_stride + tex_u) == 1) border = true;
                else if (tex_v < sheet->frame_h - 1   && atlas_px(frame_data, (tex_v + 1) * row_stride + tex_u) == 1) border = true;
                if (border) b.clear_pixel(col, row);
            }

            v_pos += v_step;
        }
    }
}

}} // namespace Engine::Raycast
