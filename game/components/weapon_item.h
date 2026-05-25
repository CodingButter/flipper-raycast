// WeaponItem — a pickup-able weapon on the floor. weapon_id identifies
// which entry in Game::WEAPONS to give the player; bob_phase is
// ticked by sys_bob to animate the floating-in-place motion.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_WEAPON_ITEM = 1u << 22; }

struct WeaponItem {
    int   weapon_id;
    float bob_phase;
};

extern WeaponItem g_weapon_items[ECS::MAX_ENTITIES];
