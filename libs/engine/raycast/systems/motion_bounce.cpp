#include "systems.h"
#include "../world_api.h"
#include "../components/bounce.h"
#include "../../ecs/components/transform.h"
#include "../../ecs/components/velocity.h"

namespace {
constexpr float MOVABLE_RADIUS = 0.12f;   // bouncers are small

// Horizontal: axis-separated wall reflection (X and Y handled independently).
constexpr float WALL_DAMPING   = 0.7f;    // 30% energy lost per wall hit
constexpr float FRICTION       = 0.985f;  // per-tick horizontal speed retention
constexpr float REST_SPEED_SQ  = 0.05f;   // below this, vel = 0

// Vertical: gravity + ground reflection.
constexpr float GRAVITY        = 12.0f;   // cells/sec² downward
constexpr float GROUND_DAMPING = 0.55f;   // 45% energy lost per ground hit
constexpr float REST_VZ        = 0.4f;    // below this absolute, vz = 0
}

// Full physics for CMP_BOUNCE entities:
//   • horizontal: axis-separated wall reflection with damping
//   • vertical: gravity accelerates downward, floor bounce on z<=0
//   • friction settles horizontal motion over time
//   • rest thresholds zero out tiny residual velocities so things stop
//
// Use exactly one motion-behavior tag per entity (SLIDE / BOUNCE / none).
void sys_motion_bounce(float dt) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_VELOCITY | ECS::CMP_BOUNCE,
        [dt](ECS::EntityId e) {
            Transform& t = g_transforms[e];
            Velocity&  v = g_velocities[e];

            // --- Horizontal: walls ---------------------------------------
            Vec2 step = v.vel * dt;

            Vec2 try_x = t.pos + Vec2{step.x, 0};
            if (Engine::Raycast::collides_with_wall(try_x, MOVABLE_RADIUS)) {
                v.vel.x = -v.vel.x * WALL_DAMPING;
            } else {
                t.pos = try_x;
            }

            Vec2 try_y = t.pos + Vec2{0, step.y};
            if (Engine::Raycast::collides_with_wall(try_y, MOVABLE_RADIUS)) {
                v.vel.y = -v.vel.y * WALL_DAMPING;
            } else {
                t.pos = try_y;
            }

            // --- Vertical: gravity + ground ------------------------------
            v.vz -= GRAVITY * dt;
            t.z  += v.vz * dt;

            if (t.z <= 0.0f) {
                t.z = 0.0f;
                if (v.vz < -REST_VZ) {
                    v.vz = -v.vz * GROUND_DAMPING;   // bounce
                } else {
                    v.vz = 0.0f;                     // resting
                }
            }

            // --- Friction + rest thresholds ------------------------------
            v.vel *= FRICTION;
            if (v.vel.sqr_length() < REST_SPEED_SQ) {
                v.vel = Vec2{0, 0};
            }
        });
}
