#include "systems.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/world_api.h"

namespace {
constexpr float BULLET_RADIUS = 0.05f;   // hits walls at point-ish accuracy
}

// Bullets move in a straight line — no slide, no bounce. If the next
// step would cross into a wall, the bullet is destroyed (impact). Misses
// expire via CMP_LIFETIME independently.
void sys_motion_bullet(float dt) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_VELOCITY | ECS::CMP_BULLET,
        [dt](ECS::EntityId e) {
            Vec2 next = g_transforms[e].pos + g_velocities[e].vel * dt;
            if (Engine::Raycast::collides_with_wall(next, BULLET_RADIUS)) {
                ECS::destroy(e);
            } else {
                g_transforms[e].pos = next;
            }
        });
}
