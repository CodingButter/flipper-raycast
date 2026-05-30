#include "systems.h"
#include "../components/components.h"
#include <math.h>

namespace {
constexpr float BOB_FREQ      = 2.5f;   // radians per second
constexpr float BOB_AMPLITUDE = 0.04f;  // small hover
constexpr float BOB_CENTER_Z  = 0.05f;  // grazes the floor at mid-bob
}

// Advances each weapon-item's bob phase and sets its world z to a
// sin-wave offset around BOB_CENTER_Z. Gives floor pickups a soft
// "floating in space" idle.
void sys_bob(float dt) {
    ECS::each(ECS::CMP_WEAPON_ITEM | ECS::CMP_TRANSFORM, [dt](ECS::EntityId e) {
        g_weapon_items[e].bob_phase += BOB_FREQ * dt;
        g_transforms[e].z = BOB_CENTER_Z + BOB_AMPLITUDE * sinf(g_weapon_items[e].bob_phase);
    });
}
