#include "flip2d.h"
#include <furi.h>
#include <string.h>

static Color to_sdk_color(Flip2DColor c) {
    switch(c) {
    case FLIP2D_WHITE: return ColorWhite;
    case FLIP2D_XOR:   return ColorXOR;
    case FLIP2D_BLACK:
    default:           return ColorBlack;
    }
}

// ---- state setters ----

void Flip2D::set_fill_style(Flip2DColor c)   { fill_style_ = c; }
void Flip2D::set_stroke_style(Flip2DColor c) { stroke_style_ = c; }

void Flip2D::set_font(const char* name) {
    if(strcmp(name, "primary") == 0)          font_ = FontPrimary;
    else if(strcmp(name, "secondary") == 0)   font_ = FontSecondary;
    else if(strcmp(name, "keyboard") == 0)    font_ = FontKeyboard;
    else if(strcmp(name, "big_numbers") == 0) font_ = FontBigNumbers;
}

void Flip2D::set_text_align(const char* a) {
    if(strcmp(a, "center") == 0)     text_align_h_ = AlignCenter;
    else if(strcmp(a, "right") == 0) text_align_h_ = AlignRight;
    else                             text_align_h_ = AlignLeft;
}

void Flip2D::set_text_baseline(const char* a) {
    if(strcmp(a, "middle") == 0)      text_align_v_ = AlignCenter;
    else if(strcmp(a, "bottom") == 0) text_align_v_ = AlignBottom;
    else                              text_align_v_ = AlignTop;
}

// ---- rectangles ----

void Flip2D::fill_rect(int x, int y, int w, int h) {
    canvas_set_color(canvas_, to_sdk_color(fill_style_));
    canvas_draw_box(canvas_, x, y, w, h);
}

void Flip2D::stroke_rect(int x, int y, int w, int h) {
    canvas_set_color(canvas_, to_sdk_color(stroke_style_));
    canvas_draw_frame(canvas_, x, y, w, h);
}

void Flip2D::clear_rect(int x, int y, int w, int h) {
    canvas_set_color(canvas_, ColorWhite);
    canvas_draw_box(canvas_, x, y, w, h);
}

// ---- text ----

void Flip2D::fill_text(const char* text, int x, int y) {
    canvas_set_color(canvas_, to_sdk_color(fill_style_));
    canvas_set_font(canvas_, font_);
    canvas_draw_str_aligned(canvas_, x, y, text_align_h_, text_align_v_, text);
}

int Flip2D::measure_text(const char* text) {
    canvas_set_font(canvas_, font_);
    return (int)canvas_string_width(canvas_, text);
}

// ---- circles ----

void Flip2D::fill_circle(int x, int y, int r) {
    canvas_set_color(canvas_, to_sdk_color(fill_style_));
    canvas_draw_disc(canvas_, x, y, r);
}

void Flip2D::stroke_circle(int x, int y, int r) {
    canvas_set_color(canvas_, to_sdk_color(stroke_style_));
    canvas_draw_circle(canvas_, x, y, r);
}

// ---- paths (eager-draw model) ----

void Flip2D::begin_path() { has_path_ = false; }

void Flip2D::move_to(int x, int y) {
    path_x_ = (int16_t)x;
    path_y_ = (int16_t)y;
    has_path_ = true;
}

void Flip2D::line_to(int x, int y) {
    if(has_path_) {
        canvas_set_color(canvas_, to_sdk_color(stroke_style_));
        canvas_draw_line(canvas_, path_x_, path_y_, x, y);
    }
    path_x_ = (int16_t)x;
    path_y_ = (int16_t)y;
    has_path_ = true;
}

void Flip2D::stroke() {}

// ---- misc ----

void Flip2D::clear_screen() { canvas_clear(canvas_); }
