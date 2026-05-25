// Slide — tag for entities whose motion against walls should clip with
// axis-separated sliding (the "hug the wall to slide along it" behavior
// the player camera uses). Processed by Engine's sys_motion_slide.
#pragma once

#include "../../ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_SLIDE = 1u << 4; }
