// Collectible — pickup entities the player can walk into. The `kind`
// field tells sys_pickups what to do on contact (add coin / heal /
// give armor). Add new kinds by appending to the enum + handling them
// in sys_pickups + adding a prefab.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_COLLECTIBLE = 1u << 17; }

enum CollectibleKind {
    COLLECTIBLE_COIN   = 0,
    COLLECTIBLE_POTION = 1,
    COLLECTIBLE_ARMOR  = 2,
};

struct Collectible {
    CollectibleKind kind;
    int             amount;   // coin: 1, potion: HP restored, armor: armor added
};

extern Collectible g_collectibles[ECS::MAX_ENTITIES];
