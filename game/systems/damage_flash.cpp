#include "systems.h"
#include "../state.h"
#include "../components/components.h"

// Tick down the post-damage flash timer and reflect it as a generic
// "color inverted" rendering hint on the AnimSprite. The renderer doesn't
// know about damage — it only knows "invert this sprite's colors" — so
// the same primitive could be used for highlight effects, friendly auras,
// or whatever else.
void sys_damage_flash(float dt) {
    ECS::each(ECS::CMP_HEALTH | ECS::CMP_ANIM_SPRITE, [dt](ECS::EntityId e) {
        if (g_healths[e].flash_timer > 0.0f) {
            g_healths[e].flash_timer -= dt;
            g_sprites[e].color_invert = true;
        } else {
            g_sprites[e].color_invert = false;
        }
    });

    // Player isn't an entity (it's the camera) so its flash lives on
    // game state. HUD reads this directly to render the screen-edge pulse.
    if (Game::player_damage_flash_timer > 0.0f) {
        Game::player_damage_flash_timer -= dt;
    }
}
