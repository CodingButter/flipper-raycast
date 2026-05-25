#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

ECS::EntityId coin(Vec2 pos) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { /*pos=*/pos,        /*z=*/0.0f };          // on the floor
    g_velocities[e] = { /*vel=*/Vec2{0,0},  /*vz=*/0.0f };          // still until magnetism kicks in
    g_sprites[e]    = { &Game::Assets::COIN, /*frame=*/0, /*timer=*/0.0f,
                        /*fps=*/10.0f, /*scale=*/0.5f, /*flip=*/false,
                        /*invert=*/false };
    g_collectibles[e] = { COLLECTIBLE_COIN, 1 };
    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE | CMP_SLIDE | CMP_COLLECTIBLE);
    return e;
}

} // namespace Prefabs
