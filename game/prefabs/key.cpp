#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Floor-bobbing key pickup. Visually mirrors weapon items so the
// "walk onto it, you get it" affordance is consistent.
ECS::EntityId key(Vec2 pos, int key_id) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { pos, /*z=*/0.3f };
    g_velocities[e] = { Vec2{0, 0}, 0.0f };
    g_sprites[e]    = { &Game::Assets::KEY, /*frame=*/0, /*timer=*/0.0f,
                        /*fps=*/0.0f, /*scale=*/0.35f,
                        /*flip=*/false, /*invert=*/false };
    g_key_items[e]  = { key_id, /*bob_phase=*/0.0f };

    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE | CMP_KEY_ITEM);
    return e;
}

} // namespace Prefabs
