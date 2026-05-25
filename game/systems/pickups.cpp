#include "systems.h"
#include "../state.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/haptic.h"

namespace {
constexpr float PICKUP_RADIUS_SQ = 0.2f * 0.2f;
}

// Dispatches collectibles by kind on contact. New pickup types: add an
// enum value in collectible.h, a prefab that sets the kind + amount,
// then a case here.
void sys_pickups(const Engine::Raycast::Camera& cam) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_COLLECTIBLE, [&](ECS::EntityId e) {
        Vec2 delta = g_transforms[e].pos - cam.position;
        if (delta.sqr_length() >= PICKUP_RADIUS_SQ) return;

        switch (g_collectibles[e].kind) {
            case COLLECTIBLE_COIN:
                Game::coins_collected += g_collectibles[e].amount;
                break;
            case COLLECTIBLE_POTION:
                Game::player_health += g_collectibles[e].amount;
                if (Game::player_health > Game::PLAYER_MAX_HEALTH)
                    Game::player_health = Game::PLAYER_MAX_HEALTH;
                Engine::Haptic::pulse(2, 30, 40);   // distinct chirp
                break;
            case COLLECTIBLE_ARMOR:
                Game::player_armor += g_collectibles[e].amount;
                if (Game::player_armor > Game::PLAYER_MAX_ARMOR)
                    Game::player_armor = Game::PLAYER_MAX_ARMOR;
                Engine::Haptic::pulse(2, 40, 40);
                break;
        }
        ECS::destroy(e);
    });
}
