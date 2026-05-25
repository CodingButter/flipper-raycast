#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Health potion — heals `amount` HP on pickup (capped at PLAYER_MAX_HEALTH).
ECS::EntityId potion(Vec2 pos, int amount) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    // z = scale/2 → bottom of sprite sits on the floor (horizon line).
    g_transforms[e]   = { pos, /*z=*/0.25f };
    g_velocities[e]   = { Vec2{0, 0}, 0.0f };
    g_sprites[e]      = { &Game::Assets::POTION, 0, 0.0f, 0.0f, 0.5f, false, false };
    g_collectibles[e] = { COLLECTIBLE_POTION, amount };
    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE | CMP_SLIDE | CMP_COLLECTIBLE);
    return e;
}

} // namespace Prefabs
