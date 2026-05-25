#include "systems.h"
#include "../state.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/haptic.h"

namespace {
constexpr float KEY_PICKUP_RADIUS_SQ = 0.5f * 0.5f;
}

// Player walks onto a key → marks it owned + small haptic chirp + entity
// destroyed. Doors with matching DOOR_LOCK will now respond.
void sys_key_pickups(const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_KEY_ITEM | ECS::CMP_TRANSFORM, [&](ECS::EntityId e) {
        Vec2 delta = cam.position - g_transforms[e].pos;
        if (delta.sqr_length() >= KEY_PICKUP_RADIUS_SQ) return;

        int kid = g_key_items[e].key_id;
        if (kid >= 0 && kid < Game::KEY_COUNT) {
            Game::keys_owned[kid] = true;
        }
        Engine::Haptic::pulse(2, 30, 40);   // distinctive chirp for keys
        ECS::destroy(e);
    });
}
