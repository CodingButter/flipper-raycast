#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Spawns a skeleton enemy at `pos` initially facing `facing` (radians).
//   walk_speed   = idle wander speed (0 = stationary, still rotates)
//   chase_speed  = movement speed when the AI has a target
//   turn_period  = seconds between idle direction changes
//   turn_angle   = radians added to facing on each idle turn
//
// Vision defaults: range = 6 cells, cone_half = π/4 (≈ 90° total FOV).
// Override via direct g_enemies[e] writes if you want sharper vision.
ECS::EntityId skeleton(Vec2 pos, float facing,
                       float walk_speed,
                       float turn_period,
                       float turn_angle) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { pos,         /*z=*/0.0f };
    g_velocities[e] = { Vec2{0,0},   /*vz=*/0.0f };
    g_sprites[e]    = { &Game::Assets::SKELETON_FRONT, /*frame=*/0, /*timer=*/0.0f,
                        /*fps=*/8.0f, /*scale=*/1.2f, /*flip=*/false,
                        /*invert=*/false };
    g_enemies[e]    = Enemy{};   // zero-init the AI / cache fields
    g_enemies[e].facing            = facing;
    g_enemies[e].walk_speed        = walk_speed;
    g_enemies[e].chase_speed       = 0.8f;            // a touch faster than wander
    g_enemies[e].vision_range      = 6.0f;
    g_enemies[e].vision_cone_half  = 0.7854f;         // π/4 → ~90° FOV
    g_enemies[e].turn_timer        = turn_period;
    g_enemies[e].turn_period       = turn_period;
    g_enemies[e].turn_angle        = turn_angle;
    g_enemies[e].last_cell_x       = (int)pos.x;
    g_enemies[e].last_cell_y       = (int)pos.y;
    g_enemies[e].attack_timer      = 3.0f;   // wind-up before first melee
    g_healths[e] = Health{/*hp=*/30, /*max_hp=*/30, /*flash_timer=*/0.0f};

    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE
         | CMP_SLIDE | CMP_HEALTH | CMP_ENEMY);
    return e;
}

} // namespace Prefabs
