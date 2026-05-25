#include "systems.h"
#include "../components/lifetime.h"

// Tick down Lifetime; destroy when it hits zero. Destroying during
// ECS::each is safe — the iteration is over a flat array and the slot
// just gets marked dead immediately.
void sys_lifetime(float dt) {
    ECS::each(ECS::CMP_LIFETIME, [dt](ECS::EntityId e) {
        g_lifetimes[e].seconds_left -= dt;
        if (g_lifetimes[e].seconds_left <= 0.0f) ECS::destroy(e);
    });
}
