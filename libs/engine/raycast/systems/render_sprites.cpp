#include "systems.h"
#include "../../ecs/components/transform.h"
#include "../../ecs/components/anim_sprite.h"
#include "../renderer_3d.h"   // draw_billboard primitive

// Iterate every entity with both a position and an animation, and feed it
// to the engine's billboard primitive. Passes z and scale through so the
// engine can render arcing/sized sprites correctly.
void sys_render_sprites(Bitmap1& b, const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_ANIM_SPRITE, [&](ECS::EntityId e) {
        Engine::Raycast::draw_billboard(
            b, cam,
            g_transforms[e].pos,
            g_transforms[e].z,
            g_sprites[e].sheet,
            g_sprites[e].frame,
            g_sprites[e].scale,
            g_sprites[e].flip_horizontal,
            g_sprites[e].color_invert);
    });
}
