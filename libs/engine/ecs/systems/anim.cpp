#include "systems.h"
#include "../components/anim_sprite.h"

// Advance every AnimSprite's frame based on its fps + dt.
void sys_anim(float dt) {
    ECS::each(ECS::CMP_ANIM_SPRITE, [dt](ECS::EntityId e) {
        AnimSprite& s = g_sprites[e];
        if (!s.sheet || s.fps <= 0.0f) return;
        s.timer += dt;
        const float period = 1.0f / s.fps;
        // while-loop catches up if dt > frame_period (dropped frame doesn't stall).
        while (s.timer >= period) {
            s.timer -= period;
            s.frame = (s.frame + 1) % s.sheet->frame_count;
        }
    });
}
