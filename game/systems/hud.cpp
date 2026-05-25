#include "systems.h"
#include "../state.h"
#include "../weapons.h"
#include "../../libs/engine/flipper_app.h"   // SCREEN_*

#include <gui/canvas.h>
#include <stdio.h>

// Each counter gets a white background box so the black text stays
// readable against textured walls in the scene behind it.
static void draw_chip(Canvas* canvas, int x, int y, int w, int h, const char* str) {
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x, y, w, h);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, x, y, w, h);
    canvas_draw_str(canvas, x + 2, y + h - 2, str);
}

// 9-pixel "+" reticle dead-center. Black arms + white center pixel.
static void draw_crosshair(Canvas* canvas) {
    const int cx = SCREEN_CENTER_X;
    const int cy = SCREEN_CENTER_Y;
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, cx - 4, cy,     cx + 4, cy);
    canvas_draw_line(canvas, cx,     cy - 4, cx,     cy + 4);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, cx, cy);
}

void sys_render_hud(Canvas* canvas) {
    canvas_set_font(canvas, FontSecondary);
    char buf[16];

    // Timer — top-right corner, MM:SS.
    int total = (int)Game::level_time;
    int mm = total / 60;
    int ss = total % 60;
    snprintf(buf, sizeof(buf), "%02d:%02d", mm, ss);
    int tw = canvas_string_width(canvas, buf) + 4;
    draw_chip(canvas, /*x=*/128 - tw, /*y=*/0, /*w=*/tw, /*h=*/10, buf);

    // Coin counter — directly left of the timer.
    snprintf(buf, sizeof(buf), "$%d", Game::coins_collected);
    int cw = canvas_string_width(canvas, buf) + 4;
    draw_chip(canvas, /*x=*/128 - tw - cw - 1, /*y=*/0,  /*w=*/cw, /*h=*/10, buf);

    // Health — bottom-left. Armor chip stacks just to the right of HP
    // when present so the player can see "buffer before HP starts to tick".
    snprintf(buf, sizeof(buf), "BR:%d", Game::player_health);
    int hw = canvas_string_width(canvas, buf) + 4;
    draw_chip(canvas, /*x=*/0, /*y=*/64 - 10, /*w=*/hw, /*h=*/10, buf);
    if (Game::player_armor > 0) {
        snprintf(buf, sizeof(buf), "A:%d", Game::player_armor);
        int aw = canvas_string_width(canvas, buf) + 4;
        draw_chip(canvas, /*x=*/hw + 1, /*y=*/64 - 10, aw, 10, buf);
    }

    // Weapon name + ammo — bottom-center. While reloading, shows "RELOAD"
    // so the player knows why pressing fire isn't shooting.
    const Game::WeaponDef& w = Game::WEAPONS[Game::active_weapon_id];
    if (Game::weapon_reload_remaining > 0.0f) {
        snprintf(buf, sizeof(buf), "%s RELOAD", w.name);
    } else {
        snprintf(buf, sizeof(buf), "%s %d/%d", w.name,
                 Game::weapon_ammo[Game::active_weapon_id], w.clip_size);
    }
    int aw = canvas_string_width(canvas, buf) + 4;
    draw_chip(canvas, SCREEN_CENTER_X - aw / 2, 64 - 10, aw, 10, buf);

    draw_crosshair(canvas);
}
