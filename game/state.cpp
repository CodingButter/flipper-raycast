#include "state.h"

namespace Game {

int   coins_collected           = 0;
int   player_health             = PLAYER_MAX_HEALTH;
int   player_armor              = 0;
int   enemies_killed            = 0;
float player_damage_flash_timer = 0.0f;

Vec2  level_goal_pos            = Vec2{0, 0};
float level_goal_radius_sq      = 0.6f * 0.6f;
float level_time                = 0.0f;
bool  level_complete            = false;
float level_complete_time       = 0.0f;
bool  inventory_open            = false;
int   inventory_cursor          = WEAPON_PISTOL;
bool  fire_lockout              = false;

bool  weapons_owned[WEAPON_COUNT] = { true, false, false };
int   weapon_ammo[WEAPON_COUNT]   = { 12, 0, 0 };
int   active_weapon_id            = WEAPON_PISTOL;
float weapon_cooldown             = 0.0f;
float weapon_reload_remaining     = 0.0f;

bool  keys_owned[KEY_COUNT]       = { false, false, false };

// DOOR_MODE / DOOR_TARGET are defined in world.cpp (where the backing
// arrays live). state.h declares them extern.

void reset_state() {
    coins_collected           = 0;
    player_health             = PLAYER_MAX_HEALTH;
    player_armor              = 0;
    player_damage_flash_timer = 0.0f;
    inventory_open            = false;
    inventory_cursor          = WEAPON_PISTOL;
    fire_lockout              = false;

    weapons_owned[WEAPON_PISTOL]  = true;
    weapons_owned[WEAPON_SMG]     = false;
    weapons_owned[WEAPON_SHOTGUN] = false;
    weapon_ammo[WEAPON_PISTOL]    = WEAPONS[WEAPON_PISTOL].clip_size;
    weapon_ammo[WEAPON_SMG]       = 0;
    weapon_ammo[WEAPON_SHOTGUN]   = 0;
    active_weapon_id              = WEAPON_PISTOL;
    weapon_cooldown               = 0.0f;
    weapon_reload_remaining       = 0.0f;

    for (int i = 0; i < KEY_COUNT; i++) keys_owned[i] = false;

    enemies_killed       = 0;
    level_time           = 0.0f;
    level_complete       = false;
    level_complete_time  = 0.0f;
    // level_goal_pos is reset by the level setup code in raycast.cpp.
}

} // namespace Game
