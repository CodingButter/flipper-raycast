#include "systems.h"
#include "../world_api.h"
#include "../components/slide.h"
#include "../../ecs/components/transform.h"
#include "../../ecs/components/velocity.h"

namespace {
constexpr float MOVABLE_RADIUS = 0.15f;   // small enough for tight gaps
}

// Wall-aware motion for CMP_SLIDE entities. Axis-separated sliding (try X
// alone, then Y alone) so a diagonal pull against a wall becomes a clean
// slide along the perpendicular axis instead of stopping dead.
//
// Different motion behaviors live in different systems — an entity should
// carry exactly one motion-behavior tag:
//   sys_motion          basic pos += vel*dt (engine ECS, no walls)
//   sys_motion_slide    wall sliding         (CMP_SLIDE)
//   sys_motion_bounce   wall reflection      (CMP_BOUNCE)
void sys_motion_slide(float dt) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_VELOCITY | ECS::CMP_SLIDE,
        [dt](ECS::EntityId e) {
            Vec2 step = g_velocities[e].vel * dt;

            Vec2 try_x = g_transforms[e].pos + Vec2{step.x, 0};
            if (!Engine::Raycast::collides_with_wall(try_x, MOVABLE_RADIUS)) {
                g_transforms[e].pos = try_x;
            }

            Vec2 try_y = g_transforms[e].pos + Vec2{0, step.y};
            if (!Engine::Raycast::collides_with_wall(try_y, MOVABLE_RADIUS)) {
                g_transforms[e].pos = try_y;
            }
        });
}
