// Game runtime state — counters and values that apply to the game as a
// whole, not to a specific entity. Lives outside the ECS because there's
// only ever one of each.
//
// Read by HUD / pickup / damage systems; reset by RaycastApp at startup.
#pragma once

#include <stdint.h>
#include "../libs/engine/vector.h"
#include "weapons.h"

namespace Game {

constexpr int PLAYER_MAX_HEALTH = 100;

extern int   coins_collected;
extern int   player_health;
extern int   player_armor;         // 0..PLAYER_MAX_ARMOR — absorbs damage before HP
extern int   enemies_killed;       // incremented by sys_bullet_damage on kill

constexpr int PLAYER_MAX_ARMOR = 100;

// Level / win-condition state. `goal_pos` is the world-space cell the
// player must reach to trigger completion; `goal_radius_sq` is the
// distance² threshold. `level_time` ticks up each frame until
// `level_complete` flips true, at which point gameplay pauses and the
// stats overlay renders.
extern Vec2  level_goal_pos;
extern float level_goal_radius_sq;
extern float level_time;
extern bool  level_complete;
extern float level_complete_time;   // snapshot of level_time at completion

// Set by sys_enemy_attack on each hit, ticked down by sys_damage_flash.
// HUD reads it to draw the brief "you got hit" full-screen flash.
extern float player_damage_flash_timer;

// Door behavior mode (parallel array to WORLD). Set per-cell at startup;
// sys_doors reads it to decide who controls the door's target openness.
constexpr uint8_t DOOR_AUTO   = 0;   // opens when player is nearby
constexpr uint8_t DOOR_MANUAL = 1;   // opens only via player OK press
extern uint8_t* DOOR_MODE;           // 0 / 1 per cell
extern uint8_t* DOOR_TARGET;         // 0 or 255 — sys_doors lerps openness toward it

// Lock state per cell. 0 = unlocked. 1+ = needs key with index (value - 1).
// Auto-locked doors stay closed until the key is owned; manual-locked
// doors refuse the toggle. Game owns the array.
extern uint8_t* DOOR_LOCK;

// One bool per key type. KEY_COUNT is the size of the array; extend it
// to add more key colors. Keys are picked up by walking onto them.
constexpr int KEY_COUNT = 3;
extern bool keys_owned[KEY_COUNT];

// True while a manual-door toggle should suppress weapon firing — set on
// the press that toggled the door, cleared when OK is released. Prevents
// SMG sprays from continuing after you bumped open a door.
extern bool  fire_lockout;

// Inventory overlay state.
extern bool  inventory_open;
extern int   inventory_cursor;          // index into WEAPONS being highlighted

// Weapon ownership + per-clip ammo + active selection + timers.
extern bool  weapons_owned[WEAPON_COUNT];
extern int   weapon_ammo[WEAPON_COUNT];
extern int   active_weapon_id;
extern float weapon_cooldown;            // seconds until can fire next shot
extern float weapon_reload_remaining;    // seconds remaining of current reload

void reset_state();

} // namespace Game
