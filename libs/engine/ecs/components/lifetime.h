// Lifetime — countdown in seconds. sys_lifetime decrements each tick and
// destroys the entity when it hits zero. Used by bullets, particles, etc.
#pragma once

#include "../ecs.h"

namespace ECS { constexpr ComponentMask CMP_LIFETIME = 1u << 3; }

struct Lifetime { float seconds_left; };

extern Lifetime g_lifetimes[ECS::MAX_ENTITIES];
