#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"
#include "../weapons.h"

namespace Prefabs {

ECS::EntityId weapon_item(Vec2 pos, int weapon_id) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    // Pick the sprite sheet for this weapon's icon. Default to pistol if
    // someone passes an unknown id.
    const Engine::SpriteSheet* sheet = &Game::Assets::PISTOL;
    switch (weapon_id) {
        case Game::WEAPON_SMG:     sheet = &Game::Assets::SMG_ICON; break;
        case Game::WEAPON_SHOTGUN: sheet = &Game::Assets::SHOTGUN;  break;
        default: break;
    }

    g_transforms[e]   = { pos, /*z=*/0.0f };               // sys_bob will animate z
    g_velocities[e]   = { Vec2{0, 0}, 0.0f };
    g_sprites[e]      = { sheet, /*frame=*/0, /*timer=*/0.0f,
                          /*fps=*/0.0f, /*scale=*/0.5f,    // half-wall tall
                          /*flip=*/false, /*invert=*/false };
    g_weapon_items[e] = { weapon_id, /*bob_phase=*/0.0f };

    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE | CMP_WEAPON_ITEM);
    return e;
}

} // namespace Prefabs
