// Engine-provided ECS components. These are the universal building blocks
// every game using this engine gets for free.
//
// Game-specific components live in game/components/ and use bits 16+ to
// stay out of the engine's reserved range.
//
// Engine bit registry (bits 0-15 reserved for engine use):
//   1u << 0   CMP_TRANSFORM       position in world space
//   1u << 1   CMP_VELOCITY        per-second movement vector
//   1u << 2   CMP_ANIM_SPRITE     sprite sheet ref + frame state
//   1u << 3   CMP_LIFETIME        countdown to destroy
//   1u << 4 .. 1u << 15           reserved for future engine components
#pragma once

#include "transform.h"
#include "velocity.h"
#include "anim_sprite.h"
#include "lifetime.h"
