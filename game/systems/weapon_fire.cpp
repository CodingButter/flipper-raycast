#include "systems.h"
#include "../state.h"
#include "../weapons.h"
#include "../prefabs/prefabs.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/raycast/world_api.h"
#include "../../libs/engine/flipper_app.h"   // Input, InputKeyOk
#include "../../libs/engine/haptic.h"

#include <math.h>

namespace {

constexpr float BULLET_SPEED       = 14.0f;
constexpr float INTERACT_RANGE     = 1.8f;   // cells
constexpr float INTERACT_STEP      = 0.2f;
constexpr int   INTERACT_HAPTIC_MS = 40;

// Walk forward from the camera in small steps. Returns the cell coords
// of the first MANUAL door found — or (-1, -1) if a regular wall blocks
// the path first / nothing in range.
bool find_manual_door_in_front(const Engine::Raycast::Camera& cam,
                                int* out_x, int* out_y) {
    using Engine::Raycast::WORLD;
    using Engine::Raycast::WORLD_COLS;
    using Engine::Raycast::WORLD_ROWS;
    if (!Game::DOOR_MODE) return false;

    const float cos_r = cosf(cam.rotation);
    const float sin_r = sinf(cam.rotation);

    for (float d = 0.3f; d <= INTERACT_RANGE; d += INTERACT_STEP) {
        float px = cam.position.x + cos_r * d;
        float py = cam.position.y + sin_r * d;
        int   cx = (int)px;
        int   cy = (int)py;
        if ((unsigned)cx >= (unsigned)WORLD_COLS) return false;
        if ((unsigned)cy >= (unsigned)WORLD_ROWS) return false;

        uint8_t cv = WORLD[cy * WORLD_COLS + cx];
        if (cv == 0) continue;
        if (Engine::Raycast::is_door(cv) &&
            Game::DOOR_MODE[cy * WORLD_COLS + cx] == Game::DOOR_MANUAL) {
            *out_x = cx;
            *out_y = cy;
            return true;
        }
        // Regular wall (or auto door) blocks the line — can't reach beyond.
        return false;
    }
    return false;
}

void toggle_manual_door(int cx, int cy) {
    if (!Game::DOOR_TARGET) return;
    int idx = cy * Engine::Raycast::WORLD_COLS + cx;

    // Lock check — if a key is required and we don't own it, refuse the
    // toggle and play a distinct double-buzz so the player knows WHY
    // nothing happened.
    if (Game::DOOR_LOCK && Game::DOOR_LOCK[idx] > 0) {
        int need = Game::DOOR_LOCK[idx] - 1;
        if (need >= Game::KEY_COUNT || !Game::keys_owned[need]) {
            Engine::Haptic::pulse(2, 60, 80);   // "denied" feedback
            return;
        }
    }

    Game::DOOR_TARGET[idx] = (Game::DOOR_TARGET[idx] > 128) ? 0 : 255;
    Engine::Haptic::blast(INTERACT_HAPTIC_MS);
}

// Find a chest in front of the player. Returns NULL_ENTITY if none in
// range. ~1.5 cell radius + 45° forward cone — close enough to be sure
// the player meant to open THIS chest, not one in the next room.
ECS::EntityId find_chest_in_front(const Engine::Raycast::Camera& cam) {
    const float cos_r = cosf(cam.rotation);
    const float sin_r = sinf(cam.rotation);
    ECS::EntityId found = ECS::NULL_ENTITY;
    float best_dist_sq = 1.5f * 1.5f;
    ECS::each(ECS::CMP_CHEST | ECS::CMP_TRANSFORM, [&](ECS::EntityId e) {
        Vec2 delta = g_transforms[e].pos - cam.position;
        float d_sq = delta.sqr_length();
        if (d_sq >= best_dist_sq) return;
        float d = sqrtf(d_sq);
        if (d < 0.001f) { found = e; return; }
        float dot = (delta.x * cos_r + delta.y * sin_r) / d;
        if (dot < 0.7f) return;       // 0.7 ≈ cos(45°)
        best_dist_sq = d_sq;
        found = e;
    });
    return found;
}

// Open a chest at the given entity — spawns its loot and destroys it.
// Loot pops out at the chest's position; magnetism + pickups handle the rest.
void open_chest(ECS::EntityId chest_id) {
    const Vec2 pos = g_transforms[chest_id].pos;
    const Chest& c = g_chests[chest_id];
    for (int i = 0; i < c.coins; i++) {
        // Spread coins slightly so they don't all stack on one spot.
        float ox = (float)((i * 17) % 7) * 0.05f - 0.15f;
        float oy = (float)((i * 11) % 5) * 0.05f - 0.10f;
        Prefabs::coin(Vec2{pos.x + ox, pos.y + oy});
    }
    if (c.has_potion) Prefabs::potion(Vec2{pos.x + 0.3f, pos.y});
    if (c.has_armor)  Prefabs::armor (Vec2{pos.x - 0.3f, pos.y});
    Engine::Haptic::pulse(3, 40, 60);   // chunky "ka-chunk" on open
    ECS::destroy(chest_id);
}

}

// Single source of truth for "what happens when you press fire."
// Handles cooldown, reload, semi-auto vs full-auto, multi-pellet shotgun
// spread, AND manual-door interaction (which preempts firing when the
// player is facing one).
void sys_weapon_fire(const Engine::Raycast::Camera& cam,
                     const Input& input, float dt) {
    // Cooldown always ticks (even during reload / lockout / aim).
    Game::weapon_cooldown -= dt;
    if (Game::weapon_cooldown < 0.0f) Game::weapon_cooldown = 0.0f;

    // Tick a reload in progress.
    if (Game::weapon_reload_remaining > 0.0f) {
        Game::weapon_reload_remaining -= dt;
        if (Game::weapon_reload_remaining <= 0.0f) {
            Game::weapon_reload_remaining = 0.0f;
            Game::weapon_ammo[Game::active_weapon_id] =
                Game::WEAPONS[Game::active_weapon_id].clip_size;
        }
        return;
    }

    // Release of OK clears the fire-lockout that a door-toggle set.
    if (input.was_released(InputKeyOk)) Game::fire_lockout = false;

    // OK-press → interact-first (door / chest), then fire.
    if (input.was_pressed(InputKeyOk)) {
        int dx, dy;
        if (find_manual_door_in_front(cam, &dx, &dy)) {
            toggle_manual_door(dx, dy);
            Game::fire_lockout = true;
            return;
        }
        ECS::EntityId c = find_chest_in_front(cam);
        if (c != ECS::NULL_ENTITY) {
            open_chest(c);
            Game::fire_lockout = true;
            return;
        }
    }
    if (Game::fire_lockout) return;

    const Game::WeaponDef& w = Game::WEAPONS[Game::active_weapon_id];
    const bool wants_fire = w.auto_fire ? input.is_pressed(InputKeyOk)
                                        : input.was_pressed(InputKeyOk);
    if (!wants_fire) return;
    if (Game::weapon_cooldown > 0.0f) return;

    int& ammo = Game::weapon_ammo[Game::active_weapon_id];
    if (ammo <= 0) {
        Game::weapon_reload_remaining = w.reload_seconds;
        return;
    }

    for (int p = 0; p < w.pellets_per_shot; p++) {
        float angle = cam.rotation;
        if (w.pellets_per_shot > 1) {
            float t = (float)p / (float)(w.pellets_per_shot - 1) * 2.0f - 1.0f;
            angle += t * w.spread_radians * 0.5f;
        }
        Vec2 forward{cosf(angle), sinf(angle)};
        Prefabs::bullet(cam.position, forward * BULLET_SPEED, w.damage_per_pellet);
    }

    ammo--;
    Game::weapon_cooldown = w.cooldown_seconds;
    if (ammo <= 0) Game::weapon_reload_remaining = w.reload_seconds;

    Engine::Haptic::blast(w.fire_haptic_ms);
}
