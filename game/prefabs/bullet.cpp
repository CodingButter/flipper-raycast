#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Small, fast projectile. damage stored on the Bullet component so each
// weapon's bullets hit for the right amount. CMP_BULLET → straight-line
// motion + destroy on wall hit; CMP_LIFETIME → misses expire on their own.
ECS::EntityId bullet(Vec2 pos, Vec2 vel, int damage) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { pos,       /*z=*/0.5f };
    g_velocities[e] = { vel,       /*vz=*/0.0f };
    g_sprites[e]    = { &Game::Assets::BULLET, /*frame=*/0, /*timer=*/0.0f,
                        /*fps=*/0.0f, /*scale=*/0.12f, /*flip=*/false,
                        /*invert=*/false };
    g_lifetimes[e].seconds_left = 1.0f;
    g_bullets[e].damage         = damage;

    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE
         | CMP_BULLET | CMP_LIFETIME);
    return e;
}

} // namespace Prefabs
