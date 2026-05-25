// Prefabs — entity factories. Each prefab function takes whatever runtime
// inputs make sense (a spawn position, a velocity, etc.), creates an
// entity with the right component combination, and returns its EntityId.
//
// Each prefab lives in its own .cpp file in this directory; this header
// is the umbrella declaring all of them.
//
// To add a prefab:
//   1. Create prefabs/<name>.cpp implementing `Prefabs::<name>(...)`.
//   2. Declare it here.
#pragma once

#include "../../libs/engine/vector.h"
#include "../../libs/engine/ecs/ecs.h"

namespace Prefabs {

ECS::EntityId coin(Vec2 pos);
ECS::EntityId bullet(Vec2 pos, Vec2 vel, int damage);
ECS::EntityId weapon_item(Vec2 pos, int weapon_id);
ECS::EntityId key(Vec2 pos, int key_id);
ECS::EntityId goal(Vec2 pos);
ECS::EntityId potion(Vec2 pos, int amount = 25);
ECS::EntityId armor (Vec2 pos, int amount = 25);
ECS::EntityId torch (Vec2 pos);   // pure decor
ECS::EntityId chest (Vec2 pos, int coins = 5, bool has_potion = false, bool has_armor = false);
ECS::EntityId skeleton(Vec2 pos, float facing,
                       float walk_speed   = 0.4f,
                       float turn_period  = 3.0f,
                       float turn_angle   = 1.5707963f);   // π/2 = 90° turns
// Future:
// ECS::EntityId enemy(Vec2 pos);

} // namespace Prefabs
