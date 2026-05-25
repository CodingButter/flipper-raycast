// Game component umbrella. Pulls in every engine-provided component AND
// every game-specific one, so callers get the full set with a single
// include.
//
// Engine ECS components own bits 0-3 (libs/engine/ecs/components/).
// Engine raycast components own bits 4-15 (libs/engine/raycast/components/).
// Game components below own bits 16+.
//
// Game bit registry:
//   1u << 16  CMP_HEALTH
//   1u << 17  CMP_COLLECTIBLE  (tag)
//   1u << 18  CMP_BULLET       (damage value; drives motion + damage systems)
//   1u << 21  CMP_ENEMY        (facing, walk_speed, turn state)
//   1u << 22  CMP_WEAPON_ITEM  (weapon_id + bob_phase — floor pickup)
//   1u << 23  CMP_KEY_ITEM     (key_id + bob_phase — floor pickup)
//   1u << 24  CMP_CHEST        (coins + flags — face+OK to open)
//   next free: 1u << 25
#pragma once

// Engine-provided ECS (Transform, Velocity, AnimSprite, Lifetime):
#include "../../libs/engine/ecs/components/components.h"
// Engine-provided raycast motion tags (Slide, Bounce):
#include "../../libs/engine/raycast/components/components.h"

// Game-specific:
#include "health.h"
#include "collectible.h"
#include "bullet.h"
#include "enemy.h"
#include "weapon_item.h"
#include "key_item.h"
#include "chest.h"
