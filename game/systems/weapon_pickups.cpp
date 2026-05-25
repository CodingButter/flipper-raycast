#include "systems.h"
#include "../state.h"
#include "../weapons.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"

namespace {
// Slightly larger than coin pickup radius — weapons are big to walk into.
constexpr float WEAPON_PICKUP_RADIUS_SQ = 0.6f * 0.6f;
}

// Player walking onto a weapon item picks it up: marks the weapon as
// owned and refills its clip. Destroys the floor pickup entity.
//
// Auto-equips if you don't already own that weapon (saves the menu trip).
void sys_weapon_pickups(const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_WEAPON_ITEM | ECS::CMP_TRANSFORM, [&](ECS::EntityId e) {
        Vec2 delta = cam.position - g_transforms[e].pos;
        if (delta.sqr_length() >= WEAPON_PICKUP_RADIUS_SQ) return;

        int wid = g_weapon_items[e].weapon_id;
        if (wid < 0 || wid >= Game::WEAPON_COUNT) { ECS::destroy(e); return; }

        const bool was_new = !Game::weapons_owned[wid];
        Game::weapons_owned[wid] = true;
        Game::weapon_ammo[wid]   = Game::WEAPONS[wid].clip_size;
        if (was_new) Game::active_weapon_id = wid;

        ECS::destroy(e);
    });
}
