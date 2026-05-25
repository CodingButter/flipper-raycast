#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Decorative torch — floor sprite, no gameplay component. Pure
// atmosphere. Place against walls or in corners to sell the dungeon feel.
ECS::EntityId torch(Vec2 pos) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { pos, /*z=*/0.35f };     // scale/2 → bottom on floor
    g_velocities[e] = { Vec2{0, 0}, 0.0f };
    g_sprites[e]    = { &Game::Assets::TORCH, 0, 0.0f, 0.0f, 0.7f, false, false };
    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE);
    return e;
}

} // namespace Prefabs
