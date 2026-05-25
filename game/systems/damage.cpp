#include "systems.h"
#include "../state.h"
#include "../components/components.h"

namespace {
constexpr float BULLET_HIT_RADIUS_SQ = 0.3f * 0.3f;
constexpr float FLASH_SECONDS        = 0.18f;
}

// Per tick, for every live bullet, scan all HP entities. If overlap:
// damage, flash, destroy bullet, kill on hp <= 0. Single hit per bullet —
// the bullet is destroyed on first impact and the next iteration's
// is_alive check shortcuts. O(bullets × hp_entities); negligible at our
// counts.
void sys_bullet_damage() {
    ECS::each(ECS::CMP_BULLET | ECS::CMP_TRANSFORM, [&](ECS::EntityId b) {
        if (!ECS::is_alive(b)) return;   // already destroyed this tick
        const Vec2 bpos = g_transforms[b].pos;

        ECS::each(ECS::CMP_HEALTH | ECS::CMP_TRANSFORM, [&](ECS::EntityId e) {
            if (!ECS::is_alive(b)) return;
            Vec2 delta = g_transforms[e].pos - bpos;
            if (delta.sqr_length() < BULLET_HIT_RADIUS_SQ) {
                g_healths[e].hp          -= g_bullets[b].damage;
                g_healths[e].flash_timer  = FLASH_SECONDS;
                if (g_healths[e].hp <= 0) {
                    Game::enemies_killed++;
                    ECS::destroy(e);
                }
                ECS::destroy(b);
            }
        });
    });
}
