#include "systems.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/raycast/world_api.h"
#include "../../libs/engine/constants.h"

#include <math.h>

namespace {

bool can_see_player(const Vec2& epos, const Enemy& en, const Vec2& ppos) {
    Vec2  to_p = ppos - epos;
    float dsq  = to_p.sqr_length();
    if (dsq > en.vision_range * en.vision_range) return false;

    float angle = atan2f(to_p.y, to_p.x);
    float rel   = angle - en.facing;
    while (rel >  PI) rel -= 2.0f * PI;
    while (rel < -PI) rel += 2.0f * PI;
    if (rel < 0) rel = -rel;
    if (rel > en.vision_cone_half) return false;

    return Engine::Raycast::has_line_of_sight(epos, ppos);
}

}

// Per-tick AI (simple version, no A*):
//   1. Update target_cell to player's cell whenever LOS catches them.
//   2. When has_target: walk straight toward the *target_cell*'s center.
//      sys_motion_slide handles wall sliding — getting stuck behind a
//      corner is an acceptable failure mode on this hardware.
//   3. When reached target and lost LOS → idle.
//
// Trade-offs vs A*: enemies can get stuck against concave walls if the
// player ducks around a corner. Costs basically nothing — one atan, a
// dot product, the LOS DDA. Fits embedded.
void sys_enemy_ai(const Engine::Raycast::Camera& cam, float dt) {
    ECS::each(ECS::CMP_ENEMY | ECS::CMP_TRANSFORM | ECS::CMP_VELOCITY,
        [&](ECS::EntityId e) {
            Enemy& en  = g_enemies[e];
            Vec2   pos = g_transforms[e].pos;
            int    cx  = (int)pos.x;
            int    cy  = (int)pos.y;

            const bool sees = can_see_player(pos, en, cam.position);
            if (sees) {
                en.target_cell_x = (int)cam.position.x;
                en.target_cell_y = (int)cam.position.y;
                en.has_target    = true;
            }

            if (en.has_target) {
                // At target's cell?
                if (cx == en.target_cell_x && cy == en.target_cell_y) {
                    if (!sees) {
                        en.has_target = false;   // lost trail → idle
                    } else {
                        // At target, still see them: face player, hold.
                        Vec2 to_p = cam.position - pos;
                        en.facing = atan2f(to_p.y, to_p.x);
                        g_velocities[e].vel = Vec2{0, 0};
                        return;
                    }
                } else {
                    // Walk straight toward target cell center. sys_motion_slide
                    // does the wall-sliding for us.
                    Vec2 tgt_pt{(float)en.target_cell_x + 0.5f,
                                (float)en.target_cell_y + 0.5f};
                    Vec2 to_tgt = tgt_pt - pos;
                    en.facing = atan2f(to_tgt.y, to_tgt.x);
                    g_velocities[e].vel = Vec2{cosf(en.facing), sinf(en.facing)} * en.chase_speed;
                    return;
                }
            }

            // Idle: existing wander.
            en.turn_timer -= dt;
            if (en.turn_timer <= 0.0f) {
                en.facing     += en.turn_angle;
                en.turn_timer  = en.turn_period;
            }
            Vec2 forward{cosf(en.facing), sinf(en.facing)};
            g_velocities[e].vel = forward * en.walk_speed;
        });
}
