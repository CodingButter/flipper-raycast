#include "bitmap.h"
#include <string.h>

void Bitmap1::clear() {
    memset(data_, 0, SIZE);
}

void Bitmap1::draw_line(int x0, int y0, int x1, int y1) {
    // Classic Bresenham — handles all 8 octants in one branch-free loop.
    int dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int dy = (y1 > y0) ? y0 - y1 : y1 - y0;   // negative on purpose
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    while (true) {
        set_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err << 1;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void Bitmap1::fill_box(int x, int y, int w, int h) {
    // Clip to the bitmap so the inner loops don't have to check bounds.
    if (x < 0)              { w += x; x = 0; }
    if (y < 0)              { h += y; y = 0; }
    if (x + w > WIDTH)      w = WIDTH  - x;
    if (y + h > HEIGHT)     h = HEIGHT - y;
    if (w <= 0 || h <= 0)   return;

    // Per-pixel for clarity. Could be byte-aligned for ~4× speedup on wide
    // boxes, but at our cell size (20px) the simple loop is plenty.
    for (int j = y; j < y + h; j++) {
        uint8_t* row = &data_[j * STRIDE];
        for (int i = x; i < x + w; i++) {
            row[i >> 3] |= (uint8_t)(1u << (i & 7));
        }
    }
}

void Bitmap1::fill_disc(int cx, int cy, int r) {
    // Midpoint circle algorithm; for each octant point we emit a horizontal
    // span across the disc so the interior gets filled (not just outlined).
    int x = 0, y = r, d = 3 - 2 * r;
    while (y >= x) {
        for (int i = cx - x; i <= cx + x; i++) set_pixel(i, cy + y);
        for (int i = cx - x; i <= cx + x; i++) set_pixel(i, cy - y);
        for (int i = cx - y; i <= cx + y; i++) set_pixel(i, cy + x);
        for (int i = cx - y; i <= cx + y; i++) set_pixel(i, cy - x);
        if (d > 0) { y--; d += 4 * (x - y) + 10; }
        else       { d += 4 * x + 6; }
        x++;
    }
}

void Bitmap1::blit(int dst_x, int dst_y, int w, int h, const uint8_t* src) {
    const int src_stride = (w + 7) / 8;
    for (int j = 0; j < h; j++) {
        int dy = dst_y + j;
        if ((unsigned)dy >= (unsigned)HEIGHT) continue;
        const uint8_t* src_row = &src[j * src_stride];
        uint8_t* dst_row = &data_[dy * STRIDE];
        for (int i = 0; i < w; i++) {
            if (src_row[i >> 3] & (uint8_t)(1u << (i & 7))) {
                int dx = dst_x + i;
                if ((unsigned)dx < (unsigned)WIDTH) {
                    dst_row[dx >> 3] |= (uint8_t)(1u << (dx & 7));
                }
            }
        }
    }
}

void Bitmap1::commit(Canvas* canvas) const {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_xbm(canvas, 0, 0, WIDTH, HEIGHT, data_);
}
