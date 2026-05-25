// Chest — interactable container. Player faces it within ~1.5 cells and
// taps OK to open; the chest spawns its loot at its position and gets
// destroyed. Loot is described by (coin_count, has_potion) for now;
// extend the struct as we add more loot kinds.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_CHEST = 1u << 24; }

struct Chest {
    int  coins;       // number of coin entities spawned on open
    bool has_potion;  // also spawns a potion next to the chest
    bool has_armor;
};

extern Chest g_chests[ECS::MAX_ENTITIES];
