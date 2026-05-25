// Camera — the player's eye + view configuration. Engine-level: every
// raycast game has one of these. position/rotation are the only fields
// driven by input each tick; the rest (fov/clips/radius/speeds) are config
// set at construction. pl/pr/fl/fr are derived (view-frustum corners) and
// refreshed by move_camera → update_view_frustum every tick.
#pragma once

#include "../vector.h"
#include "../flipper_app.h"   // Input

namespace Engine { namespace Raycast {

struct Camera {
    Vec2  position;          // grid coord of the eye
    float rotation;          // radians; 0 = facing +x, increasing = CW on screen
    float radius;            // collision footprint (grid units)
    float speed;             // grid cells per second along facing
    float rotation_speed;    // BASE radians/sec (multiplied by exponential ramp)
    float fov;               // radians
    float near_clip;
    float far_clip;
    Vec2 pl{}, pr{};         // near-plane corners (auto-updated)
    Vec2 fl{}, fr{};         // far-plane  corners (auto-updated)
    // Exponential-turn book-keeping. Held longer = faster, up to a cap;
    // releasing or switching direction resets. Auto-tracked by move_camera.
    float turn_L_held_time = 0.0f;
    float turn_R_held_time = 0.0f;
};

void update_view_frustum(Camera& camera);

// Reads input, applies movement + axis-separated wall sliding, refreshes
// the frustum. Returns false to signal the app should exit (currently
// always true — kept for future use).
bool move_camera(const Input& input, Camera& camera, float dt);

}} // namespace Engine::Raycast
