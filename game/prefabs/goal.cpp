#include "prefabs.h"
#include "../components/components.h"
#include "../assets/assets.h"

namespace Prefabs {

// Visual marker for the level's exit. Doesn't carry any gameplay
// component — sys_level just checks distance from the player to
// Game::level_goal_pos. This entity exists purely so the player can
// SEE where to go.
ECS::EntityId goal(Vec2 pos) {
    using namespace ECS;
    EntityId e = create();
    if (e == NULL_ENTITY) return e;

    g_transforms[e] = { pos, /*z=*/0.30f };     // scale/2 → bottom on floor
    g_velocities[e] = { Vec2{0, 0}, 0.0f };
    g_sprites[e]    = { &Game::Assets::GOAL, /*frame=*/0, /*timer=*/0.0f,
                        /*fps=*/0.0f, /*scale=*/0.6f,
                        /*flip=*/false, /*invert=*/false };

    add(e, CMP_TRANSFORM | CMP_VELOCITY | CMP_ANIM_SPRITE);
    return e;
}

} // namespace Prefabs
