// Bounce — tag for entities that reflect off walls (axis-separated; the X
// or Y velocity component flips on a hit, with energy damping each bounce).
// Processed by Engine's sys_motion_bounce.
#pragma once

#include "../../ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_BOUNCE = 1u << 5; }
