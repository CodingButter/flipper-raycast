// Weapon definitions — a single constexpr table indexed by WeaponId.
// Add new weapons by appending a row + bumping WEAPON_COUNT.
//
// pellets_per_shot > 1 is shotgun-style: pellets evenly fan across
// `spread_radians`. For single-shot weapons (pistol/SMG), spread is
// usually 0 — bullets fly perfectly straight.
//
// auto_fire = true means holding OK keeps firing at `cooldown_seconds`
// intervals; false means each shot needs a fresh OK press.
#pragma once

namespace Game {

enum WeaponId : int {
    WEAPON_PISTOL  = 0,
    WEAPON_SMG     = 1,
    WEAPON_SHOTGUN = 2,
    WEAPON_COUNT   = 3
};

struct WeaponDef {
    const char* name;
    int   clip_size;
    int   damage_per_pellet;
    int   pellets_per_shot;
    float spread_radians;
    float cooldown_seconds;
    float reload_seconds;
    bool  auto_fire;
    int   fire_haptic_ms;     // vibration duration per shot
};

extern const WeaponDef WEAPONS[WEAPON_COUNT];

} // namespace Game
