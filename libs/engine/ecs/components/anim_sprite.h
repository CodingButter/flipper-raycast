// AnimSprite — references a sprite sheet and tracks animation playback
// (current frame + accumulator). Drawn by sys_render_sprites, advanced by
// sys_anim.
//
// `scale` is a world-size multiplier: 1.0 = 1 grid unit tall (default for
// most things), 0.25 = quarter the size (good for particles), 2.0 = twice
// the size (giant boss). It multiplies the projected pixel size in the
// renderer, so distance still controls the on-screen size.
#pragma once

#include "../ecs.h"
#include "../../assets/assets.h"   // Engine::SpriteSheet

namespace ECS { constexpr ComponentMask CMP_ANIM_SPRITE = 1u << 2; }

struct AnimSprite {
    const Engine::SpriteSheet* sheet;
    int   frame;
    float timer;            // seconds accumulator (advanced by sys_anim)
    float fps;              // animation rate; 0 = static (frame never advances)
    float scale;            // world-size multiplier; 1.0 = 1 cell tall
    bool  flip_horizontal;  // mirror U sampling — for direction sprites with
                            // mirrored angles (e.g. enemy's left = right mirrored)
    bool  color_invert;     // swap 1↔2 during sampling — for damage-flash pulses
                            // and other generic "this thing is highlighted" effects
};

extern AnimSprite g_sprites[ECS::MAX_ENTITIES];
