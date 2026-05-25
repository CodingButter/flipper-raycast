// Transform — world position for any entity that exists somewhere in
// the level. Required by motion, rendering, collision, etc.
//
// `z` is the vertical (out-of-floor) coordinate in grid units. z = 0
// means "on the floor / at eye level" — the default for most entities
// (coins, anything stuck to the ground). Bombs / particles use z > 0
// during flight; sys_motion_bounce + sys_render_sprites both understand
// z so arcing objects render at the correct height.
#pragma once

#include "../ecs.h"
#include "../../vector.h"

namespace ECS { constexpr ComponentMask CMP_TRANSFORM = 1u << 0; }

struct Transform {
    Vec2  pos;
    float z;
};

extern Transform g_transforms[ECS::MAX_ENTITIES];
