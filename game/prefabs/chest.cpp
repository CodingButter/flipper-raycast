#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Treasure chest — face it and tap OK to open. Loot params decide what
// spawns. `coins` = number of coins; `has_potion` / `has_armor` add a
// potion or armor pickup next to the spawn point.
ECS::EntityId chest(Vec2 pos, int coins, bool has_potion, bool has_armor) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { pos, /*z=*/0.30f };     // scale/2 → bottom on floor
    g_velocities[e] = { Vec2{0, 0}, 0.0f };
    g_sprites[e]    = { &Game::Assets::CHEST, 0, 0.0f, 0.0f, 0.6f, false, false };
    g_chests[e]     = { coins, has_potion, has_armor };
    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE | CMP_CHEST);
    return e;
}

} // namespace Prefabs
