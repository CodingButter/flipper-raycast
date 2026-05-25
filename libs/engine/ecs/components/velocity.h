// Velocity — grid-units-per-second movement vector. Paired with Transform
// by sys_motion to make things move.
//
// `vz` is the vertical component. sys_motion_bounce applies gravity to it
// and bounces off the floor (z = 0); sys_motion / sys_motion_slide only
// touch the horizontal vector, leaving any vz untouched.
#pragma once

#include "../ecs.h"
#include "../../vector.h"

namespace ECS { constexpr ComponentMask CMP_VELOCITY = 1u << 1; }

struct Velocity {
    Vec2  vel;
    float vz;
};

extern Velocity g_velocities[ECS::MAX_ENTITIES];
