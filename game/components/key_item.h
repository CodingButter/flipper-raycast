// KeyItem — a pickup-able key. `key_id` indexes into Game::keys_owned
// (one bool per key type). bob_phase animates the floor-floating motion
// just like WeaponItem does.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_KEY_ITEM = 1u << 23; }

struct KeyItem {
    int   key_id;
    float bob_phase;
};

extern KeyItem g_key_items[ECS::MAX_ENTITIES];
