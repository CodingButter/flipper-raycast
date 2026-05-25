#include "systems.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include <math.h>

namespace {
// Player has a ~1.2-cell radius of influence. A coin inside it gets a
// velocity vector pointing at the player; outside, velocity is zeroed
// so coins sit still until the player approaches.
constexpr float INFLUENCE_RADIUS = 1.2f;
constexpr float INFLUENCE_SQ     = INFLUENCE_RADIUS * INFLUENCE_RADIUS;

// Peak pull speed (cells/sec) when a coin is right on top of the player.
// Pull magnitude scales as (1 - dist/INFLUENCE)² — slow at the rim,
// snappy near the center. That quadratic ramp is what reads as "gravity."
constexpr float MAX_PULL_SPEED = 8.0f;
}

// Set per-coin velocity each tick. sys_motion does the actual movement
// next; sys_pickups catches the overlap a tick or two later.
void sys_magnetism(const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_VELOCITY | ECS::CMP_COLLECTIBLE,
        [&](ECS::EntityId e) {
            Vec2  delta = cam.position - g_transforms[e].pos;
            float dsq   = delta.sqr_length();
            if (dsq < INFLUENCE_SQ && dsq > 0.0001f) {
                float dist = sqrtf(dsq);
                float pull = 1.0f - dist / INFLUENCE_RADIUS;
                pull *= pull;                          // quadratic falloff
                Vec2  dir  = delta * (1.0f / dist);    // unit vector toward player
                g_velocities[e].vel = dir * (MAX_PULL_SPEED * pull);
            } else {
                g_velocities[e].vel = Vec2{0, 0};       // out of range — sit still
            }
        });
}
