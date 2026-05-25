#include "systems.h"
#include "../components/components.h"
#include "../assets/assets.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/constants.h"
#include <math.h>

namespace {

// 8 octants, each π/4 wide. `rel` = angle from enemy's facing toward the
// camera. rel ≈ 0 means the camera is in front of the enemy, so the
// player sees the FRONT sprite. rel ≈ ±π means camera is behind → BACK.
//
// Negative rel values mirror their positive counterparts (the engine's
// flip_horizontal flag flips U sampling at draw time).
void pick_sheet_and_flip(float rel,
                          const Engine::SpriteSheet*& sheet,
                          bool& flip)
{
    using namespace Game::Assets;
    if      (rel < -7*PI/8) { sheet = &SKELETON_BACK;       flip = false; }
    else if (rel < -5*PI/8) { sheet = &SKELETON_BACK_LEFT;  flip = true;  }
    else if (rel < -3*PI/8) { sheet = &SKELETON_RIGHT;      flip = true;  }
    else if (rel <   -PI/8) { sheet = &SKELETON_FRONT_LEFT; flip = true;  }
    else if (rel <    PI/8) { sheet = &SKELETON_FRONT;      flip = false; }
    else if (rel <  3*PI/8) { sheet = &SKELETON_FRONT_LEFT; flip = false; }
    else if (rel <  5*PI/8) { sheet = &SKELETON_RIGHT;      flip = false; }
    else if (rel <  7*PI/8) { sheet = &SKELETON_BACK_LEFT;  flip = false; }
    else                    { sheet = &SKELETON_BACK;       flip = false; }
}

}

// For each enemy: compute the angle from the enemy's facing direction
// toward the camera, then swap the AnimSprite to the matching directional
// sheet (and set flip_horizontal for the mirrored angles).
//
// The walk-cycle phase (AnimSprite::frame) is NOT touched here — sys_anim
// keeps advancing it. All five SKELETON_* sheets share frame_count = 10,
// so a mid-stride direction swap continues the cycle seamlessly.
void sys_enemy_facing(const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_ENEMY | ECS::CMP_TRANSFORM | ECS::CMP_ANIM_SPRITE,
        [&](ECS::EntityId e) {
            Vec2  to_cam       = cam.position - g_transforms[e].pos;
            float angle_to_cam = atan2f(to_cam.y, to_cam.x);
            float rel          = angle_to_cam - g_enemies[e].facing;
            // Normalize to [-π, π]
            while (rel >  PI) rel -= 2.0f * PI;
            while (rel < -PI) rel += 2.0f * PI;

            const Engine::SpriteSheet* sheet;
            bool flip;
            pick_sheet_and_flip(rel, sheet, flip);

            g_sprites[e].sheet           = sheet;
            g_sprites[e].flip_horizontal = flip;
        });
}
