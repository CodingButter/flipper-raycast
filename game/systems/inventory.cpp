#include "systems.h"
#include "../state.h"
#include "../weapons.h"
#include "../../libs/engine/flipper_app.h"

#include <gui/canvas.h>
#include <stdio.h>

namespace {

// Move the cursor to the next OWNED weapon in `dir` (-1 = up, +1 = down),
// wrapping around. If no other weapon is owned, leave cursor put.
void move_cursor(int dir) {
    for (int n = 1; n < Game::WEAPON_COUNT; n++) {
        int idx = (Game::inventory_cursor + dir * n
                   + Game::WEAPON_COUNT * 2) % Game::WEAPON_COUNT;
        if (Game::weapons_owned[idx]) {
            Game::inventory_cursor = idx;
            return;
        }
    }
}

}

// Process inventory navigation. Only called from raycast.cpp when the
// inventory is open. OK equips the highlighted weapon (cancels any
// in-progress reload so the swap feels instant).
void sys_inventory_input(const Input& input) {
    if (input.was_pressed(InputKeyUp))   move_cursor(-1);
    if (input.was_pressed(InputKeyDown)) move_cursor(+1);

    if (input.was_pressed(InputKeyOk)) {
        if (Game::weapons_owned[Game::inventory_cursor]) {
            Game::active_weapon_id        = Game::inventory_cursor;
            Game::weapon_reload_remaining = 0.0f;   // swap cancels reload
            Game::weapon_cooldown         = 0.0f;
        }
    }
}

// Renders the inventory panel — full-screen black with weapon list and
// the keybind hints. Owned weapons show name + clip; the selected row
// gets a leading ">" cursor and the equipped weapon gets a trailing "*".
void sys_render_inventory(Canvas* canvas) {
    if (!Game::inventory_open) return;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box  (canvas, 0, 0, SCREEN_WIDTH,     SCREEN_HEIGHT);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_frame(canvas, 0, 0, SCREEN_WIDTH,     SCREEN_HEIGHT);
    canvas_draw_frame(canvas, 1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, SCREEN_CENTER_X, 4,
                            AlignCenter, AlignTop, "INVENTORY");
    canvas_draw_line(canvas, 18, 13, SCREEN_WIDTH - 18, 13);

    // Weapon list — only owned weapons; cursor + equipped markers.
    int y = 22;
    for (int i = 0; i < Game::WEAPON_COUNT; i++) {
        if (!Game::weapons_owned[i]) continue;
        const Game::WeaponDef& w = Game::WEAPONS[i];
        char prefix = (i == Game::inventory_cursor)  ? '>' : ' ';
        char marker = (i == Game::active_weapon_id) ? '*' : ' ';
        char buf[32];
        snprintf(buf, sizeof(buf), "%c%c %-7s %2d/%-2d",
                 prefix, marker, w.name, Game::weapon_ammo[i], w.clip_size);
        canvas_draw_str(canvas, 4, y, buf);
        y += 9;
    }

    // Keys row — show which keys the player has collected. Owned keys
    // print their index; missing keys show `.` so the player can see how
    // many key types exist overall.
    y += 2;
    char keys_buf[32];
    int  written = snprintf(keys_buf, sizeof(keys_buf), "KEYS: ");
    for (int i = 0; i < Game::KEY_COUNT && written < (int)sizeof(keys_buf) - 2; i++) {
        keys_buf[written++] = Game::keys_owned[i] ? ('1' + i) : '.';
        keys_buf[written++] = ' ';
    }
    keys_buf[written] = '\0';
    canvas_draw_str(canvas, 4, y, keys_buf);

    // Hint at the bottom.
    canvas_draw_str(canvas, 4, 60, "OK eq  BACK close");
}
