#include "systems.h"
#include "../state.h"
#include "../components/components.h"
#include "../../libs/engine/raycast/camera.h"
#include "../../libs/engine/haptic.h"

namespace {
constexpr float ATTACK_RANGE_SQ  = 0.9f * 0.9f;   // ~1 cell melee reach
constexpr int   ATTACK_DAMAGE    = 5;
constexpr float ATTACK_COOLDOWN  = 3.0f;          // seconds between pulses
constexpr float FLASH_SECONDS    = 0.12f;         // ~2 visible frames @ 15 Hz draw
}

// For every enemy: tick attack_timer. If the player is inside melee
// range and the cooldown has expired, deal damage + reset the cooldown.
//
// The cooldown ticks REGARDLESS of range — so an enemy that's been
// waiting outside range can hit you immediately on contact. If you'd
// rather "first hit takes ATTACK_COOLDOWN after entering range," reset
// the timer in the out-of-range branch.
void sys_enemy_attack(const Engine::Raycast::Camera& cam, float dt) {
    ECS::each(ECS::CMP_ENEMY | ECS::CMP_TRANSFORM, [&](ECS::EntityId e) {
        Enemy& en  = g_enemies[e];
        en.attack_timer -= dt;
        if (en.attack_timer < 0.0f) en.attack_timer = 0.0f;

        Vec2  delta = cam.position - g_transforms[e].pos;
        if (delta.sqr_length() > ATTACK_RANGE_SQ) return;
        if (en.attack_timer > 0.0f) return;

        // Armor absorbs damage first. Once depleted, overflow bleeds into HP.
        int dmg = ATTACK_DAMAGE;
        if (Game::player_armor > 0) {
            int absorbed = (dmg < Game::player_armor) ? dmg : Game::player_armor;
            Game::player_armor -= absorbed;
            dmg              -= absorbed;
        }
        Game::player_health -= dmg;
        if (Game::player_health < 0) Game::player_health = 0;
        Game::player_damage_flash_timer = FLASH_SECONDS;
        Engine::Haptic::blast(200);

        en.attack_timer = ATTACK_COOLDOWN;
    });
}
