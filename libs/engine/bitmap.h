// ---------------------------------------------------------------------------
// Bitmap1 — 128×64 monochrome offscreen buffer for fast batched drawing.
//
// The Flipper SDK doesn't expose its internal framebuffer pointer, so the
// closest we can get to "draw straight to memory" is to maintain our own
// 1-bit bitmap, draw into it with tight inline pixel writes (no function-call
// overhead per pixel), and blit the whole thing onto the canvas in ONE
// canvas_draw_xbm call.
//
// Why this beats 100+ canvas_draw_line calls:
//   • Each line we draw is pure integer math + a byte-mask OR — typically
//     ~3 cycles per pixel inlined, no SDK call overhead.
//   • The commit step is one canvas call instead of one per primitive, so
//     SDK-side state setup (canvas_set_color etc.) happens once per frame.
//
// Coordinate system matches the canvas: (0,0) top-left, +x right, +y down.
// XBM storage layout: row-major, 16 bytes per row, LSB = leftmost pixel in
// the byte. canvas_draw_xbm consumes this format directly.
// ---------------------------------------------------------------------------

#pragma once
#include <gui/gui.h>
#include <stdint.h>

class Bitmap1 {
public:
    static constexpr int WIDTH  = 128;
    static constexpr int HEIGHT = 64;
    static constexpr int STRIDE = (WIDTH + 7) / 8;     // 16 bytes per row
    static constexpr int SIZE   = STRIDE * HEIGHT;     // 1024 bytes total

    // Wipe the buffer. memset of 1024 bytes — ~250 cycles on Cortex-M4.
    void clear();

    // Inline single-pixel write. Out-of-bounds writes are silently dropped
    // via an unsigned compare (catches negative and >= W/H in one branch).
    void set_pixel(int x, int y) {
        if ((unsigned)x >= (unsigned)WIDTH)  return;
        if ((unsigned)y >= (unsigned)HEIGHT) return;
        data_[y * STRIDE + (x >> 3)] |= (uint8_t)(1u << (x & 7));
    }

    // Force a pixel back to "off" (canvas-white on screen). Used by sprite
    // rendering to punch white pixels through walls drawn behind the sprite.
    void clear_pixel(int x, int y) {
        if ((unsigned)x >= (unsigned)WIDTH)  return;
        if ((unsigned)y >= (unsigned)HEIGHT) return;
        data_[y * STRIDE + (x >> 3)] &= (uint8_t)~(1u << (x & 7));
    }

    // Bresenham line — pure integer math, calls inline set_pixel per pixel.
    void draw_line(int x0, int y0, int x1, int y1);

    // Filled axis-aligned box. Clips to the bitmap.
    void fill_box(int x, int y, int w, int h);

    // Filled disc (midpoint circle). cx/cy is the center, r the radius.
    void fill_disc(int cx, int cy, int r);

    // Paste a 1-bit XBM sprite at (dst_x, dst_y). Source must be row-major,
    // LSB-first, with stride = ceil(w/8) bytes per row. 1-bits in the source
    // draw as black; 0-bits leave the destination untouched (transparent).
    // Clips against the bitmap bounds.
    void blit(int dst_x, int dst_y, int w, int h, const uint8_t* src);

    // Commit the buffer onto the canvas in a single call. Bits set in the
    // bitmap draw as black; unset bits are transparent, so whatever the
    // canvas already contained shows through. Call canvas_clear() first if
    // you want a clean frame.
    void commit(Canvas* canvas) const;

    const uint8_t* data() const { return data_; }

private:
    uint8_t data_[SIZE] = {};
};
