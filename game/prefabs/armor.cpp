#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Armor pickup — adds `amount` to player_armor (capped at PLAYER_MAX_ARMOR).
// Armor absorbs damage before HP, so picking these up effectively extends
// your total survivable hits.
ECS::EntityId armor(Vec2 pos, int amount) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e]   = { pos, /*z=*/0.25f };   // scale/2 → bottom on floor
    g_velocities[e]   = { Vec2{0, 0}, 0.0f };
    g_sprites[e]      = { &Game::Assets::ARMOR, 0, 0.0f, 0.0f, 0.5f, false, false };
    g_collectibles[e] = { COLLECTIBLE_ARMOR, amount };
    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE | CMP_SLIDE | CMP_COLLECTIBLE);
    return e;
}

} // namespace Prefabs
