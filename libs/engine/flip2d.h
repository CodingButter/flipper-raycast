// ---------------------------------------------------------------------------
// Flip2D — HTML5-Canvas-style drawing wrapper for the Flipper Canvas.
//
// Construct with the Canvas* you got from a ViewPort draw callback, then
// call methods on it directly. Designed to live on the stack — the
// FlipperApp framework constructs one for you per draw() call.
// ---------------------------------------------------------------------------

#pragma once
#include <gui/gui.h>

enum Flip2DColor {
    FLIP2D_BLACK,
    FLIP2D_WHITE,
    FLIP2D_XOR,
};

class Flip2D {
public:
    explicit Flip2D(Canvas* canvas) : canvas_(canvas) {}

    // State
    void set_fill_style(Flip2DColor color);
    void set_stroke_style(Flip2DColor color);
    // font names: "primary" | "secondary" | "keyboard" | "big_numbers"
    void set_font(const char* font_name);
    // align: "left" | "center" | "right"
    void set_text_align(const char* align);
    // baseline: "top" | "middle" | "bottom"
    void set_text_baseline(const char* baseline);

    // Rectangles
    void fill_rect(int x, int y, int w, int h);
    void stroke_rect(int x, int y, int w, int h);
    void clear_rect(int x, int y, int w, int h);

    // Text
    void fill_text(const char* text, int x, int y);
    int  measure_text(const char* text);

    // Circles
    void fill_circle(int x, int y, int radius);
    void stroke_circle(int x, int y, int radius);

    // Paths (eager: line_to draws immediately; stroke() is a no-op)
    void begin_path();
    void move_to(int x, int y);
    void line_to(int x, int y);
    void stroke();

    // Whole-screen
    void clear_screen();

    // Escape hatch: raw underlying canvas for tight hot loops that need to
    // bypass the per-call set_color/state-bookkeeping that Flip2D layers on.
    Canvas* raw_canvas() const { return canvas_; }

private:
    Canvas* canvas_;
    Flip2DColor fill_style_   = FLIP2D_BLACK;
    Flip2DColor stroke_style_ = FLIP2D_BLACK;
    Font font_                = FontPrimary;
    Align text_align_h_       = AlignLeft;
    Align text_align_v_       = AlignTop;
    int16_t path_x_           = 0;
    int16_t path_y_           = 0;
    bool has_path_            = false;
};
