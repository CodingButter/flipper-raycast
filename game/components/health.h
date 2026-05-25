// Health — hit-point counter + max value + damage-flash timer.
//   hp          current health; entity destroyed by damage systems at ≤ 0
//   max_hp      reference for "10-chunk health bar" rendering and reset logic
//   flash_timer seconds remaining of the post-hit "color-pulse" effect
//
// sys_bullet_damage / sys_bombs decrement hp + set flash_timer.
// sys_damage_flash ticks flash_timer and toggles AnimSprite.color_invert.
// sys_render_healthbars draws the 10-chunk bar above each entity.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_HEALTH = 1u << 16; }

struct Health {
    int   hp;
    int   max_hp;
    float flash_timer;
};

extern Health g_healths[ECS::MAX_ENTITIES];
