// Bullet — tag + damage. CMP_BULLET drives sys_motion_bullet (straight-
// line, destroy on wall hit). Damage is per-bullet so different weapons
// can shoot bullets that hit for different amounts.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_BULLET = 1u << 18; }

struct Bullet { int damage; };

extern Bullet g_bullets[ECS::MAX_ENTITIES];
