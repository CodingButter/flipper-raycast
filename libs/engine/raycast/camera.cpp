#include "camera.h"
#include "world_api.h"        // collides_with_wall
#include "../constants.h"
#include <math.h>

namespace Engine { namespace Raycast {

void update_view_frustum(Camera& camera) {
    const float half_fov = camera.fov * 0.5f;
    const float pl_angle = camera.rotation - half_fov;
    const float pr_angle = camera.rotation + half_fov;
    const float inv_cos  = 1.0f / cosf(half_fov);

    // Reuse the two side-direction unit vectors for both near and far corners.
    const Vec2  dir_l{cosf(pl_angle), sinf(pl_angle)};
    const Vec2  dir_r{cosf(pr_angle), sinf(pr_angle)};
    const float n_dist = camera.near_clip * inv_cos;
    const float f_dist = camera.far_clip  * inv_cos;

    camera.pl = camera.position + dir_l * n_dist;
    camera.pr = camera.position + dir_r * n_dist;
    camera.fl = camera.position + dir_l * f_dist;
    camera.fr = camera.position + dir_r * f_dist;
}

bool move_camera(const Input& input, Camera& camera, float dt) {
    // Exponential turn ramp: hold longer ⇒ faster, capped at PEAK_MULT × base.
    // L and R timers are tracked independently so flipping direction starts
    // the new side from base speed (good for precision aim after a fast 180°).
    constexpr float TURN_RAMP_TIME = 0.6f;   // seconds to reach peak from base
    constexpr float TURN_PEAK_MULT = 4.0f;   // peak speed = base × this

    const bool L = input.is_pressed(InputKeyLeft);
    const bool R = input.is_pressed(InputKeyRight);
    camera.turn_L_held_time = L ? camera.turn_L_held_time + dt : 0.0f;
    camera.turn_R_held_time = R ? camera.turn_R_held_time + dt : 0.0f;

    auto mult_from_held = [](float held) {
        float r = held / TURN_RAMP_TIME;
        if (r > 1.0f) r = 1.0f;
        return powf(TURN_PEAK_MULT, r);   // 1×..PEAK_MULT × across the ramp
    };
    if (L) camera.rotation -= camera.rotation_speed * mult_from_held(camera.turn_L_held_time) * dt;
    if (R) camera.rotation += camera.rotation_speed * mult_from_held(camera.turn_R_held_time) * dt;

    float forward_input = 0;
    if (input.is_pressed(InputKeyUp))   forward_input += 1;
    if (input.is_pressed(InputKeyDown)) forward_input -= 1;

    Vec2 forward{cosf(camera.rotation), sinf(camera.rotation)};
    Vec2 step = forward * (forward_input * camera.speed * dt);

    // Axis-separated wall sliding.
    Vec2 try_x = camera.position + Vec2{step.x, 0};
    if (!collides_with_wall(try_x, camera.radius)) camera.position = try_x;

    Vec2 try_y = camera.position + Vec2{0, step.y};
    if (!collides_with_wall(try_y, camera.radius)) camera.position = try_y;

    update_view_frustum(camera);
    return true;
}

}} // namespace Engine::Raycast
