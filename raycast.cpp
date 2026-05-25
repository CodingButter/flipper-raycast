// Raycast — entry point and top-level orchestration.
//
// libs/engine/   — generic raycast engine (ECS, assets, raycast primitives)
// game/          — content (assets, components, systems, prefabs, world)
// raycast.cpp    — picks rates, owns the Camera, spawns starting prefabs,
//                  routes tick/draw through the right systems.

#include <math.h>

#include "libs/engine/flipper_app.h"
#include "libs/engine/bitmap.h"
#include "libs/engine/constants.h"               // PI, HALF_PI
#include "libs/engine/ecs/ecs.h"
#include "libs/engine/raycast/camera.h"
#include "libs/engine/raycast/ray.h"
#include "libs/engine/raycast/renderer_3d.h"
// #include "libs/engine/raycast/renderer_2d.h"  // uncomment for top-down debug

#include "game/systems/systems.h"
#include "game/prefabs/prefabs.h"
#include "game/state.h"
#include "game/weapons.h"

class RaycastApp : public FlipperApp {
public:
    RaycastApp() : FlipperApp(30, 2) {
        setup_level();
    }

private:
    // Called from the constructor and from the OK-on-stats-screen restart
    // path. Wipes ECS + game state, then re-spawns every entity, sets the
    // goal, and resets the camera. Single source of truth for "what does
    // this level look like."
    void setup_level() {
        ECS::reset();
        Game::reset_state();

        // Hand-crafted — 50 skeletons, 56 coins, verified chokepoint
        camera.position = Vec2{4.5f, 4.5f};
        camera.rotation = 0.0f;
        Engine::Raycast::update_view_frustum(camera);
        Game::level_goal_pos       = Vec2{58.5f, 58.5f};
        Game::level_goal_radius_sq = 0.6f * 0.6f;

        Prefabs::key        (Vec2{4.5f, 32.5f}, /*key_id=*/0);
        Prefabs::coin       (Vec2{3.5f, 2.5f});
        Prefabs::coin       (Vec2{5.5f, 2.5f});
        Prefabs::coin       (Vec2{3.5f, 5.5f});
        Prefabs::coin       (Vec2{5.5f, 5.5f});
        Prefabs::coin       (Vec2{3.5f, 9.5f});
        Prefabs::coin       (Vec2{11.5f, 3.5f});
        Prefabs::coin       (Vec2{13.5f, 3.5f});
        Prefabs::coin       (Vec2{17.5f, 3.5f});
        Prefabs::coin       (Vec2{22.5f, 3.5f});
        Prefabs::coin       (Vec2{27.5f, 3.5f});
        Prefabs::coin       (Vec2{11.5f, 11.5f});
        Prefabs::coin       (Vec2{13.5f, 11.5f});
        Prefabs::coin       (Vec2{8.5f, 13.5f});
        Prefabs::coin       (Vec2{10.5f, 14.5f});
        Prefabs::coin       (Vec2{9.5f, 19.5f});
        Prefabs::coin       (Vec2{9.5f, 21.5f});
        Prefabs::coin       (Vec2{15.5f, 19.5f});
        Prefabs::coin       (Vec2{20.5f, 19.5f});
        Prefabs::coin       (Vec2{15.5f, 21.5f});
        Prefabs::coin       (Vec2{20.5f, 21.5f});
        Prefabs::coin       (Vec2{3.5f, 13.5f});
        Prefabs::coin       (Vec2{3.5f, 18.5f});
        Prefabs::coin       (Vec2{3.5f, 22.5f});
        Prefabs::coin       (Vec2{3.5f, 26.5f});
        Prefabs::coin       (Vec2{5.5f, 31.5f});
        Prefabs::coin       (Vec2{6.5f, 33.5f});
        Prefabs::coin       (Vec2{2.5f, 32.5f});
        Prefabs::coin       (Vec2{10.5f, 31.5f});
        Prefabs::coin       (Vec2{13.5f, 31.5f});
        Prefabs::coin       (Vec2{10.5f, 33.5f});
        Prefabs::coin       (Vec2{13.5f, 33.5f});
        Prefabs::coin       (Vec2{25.5f, 29.5f});
        Prefabs::coin       (Vec2{28.5f, 29.5f});
        Prefabs::coin       (Vec2{25.5f, 31.5f});
        Prefabs::coin       (Vec2{28.5f, 31.5f});
        Prefabs::coin       (Vec2{34.5f, 29.5f});
        Prefabs::coin       (Vec2{36.5f, 29.5f});
        Prefabs::coin       (Vec2{34.5f, 31.5f});
        Prefabs::coin       (Vec2{41.5f, 29.5f});
        Prefabs::coin       (Vec2{43.5f, 29.5f});
        Prefabs::coin       (Vec2{49.5f, 28.5f});
        Prefabs::coin       (Vec2{53.5f, 28.5f});
        Prefabs::coin       (Vec2{49.5f, 31.5f});
        Prefabs::coin       (Vec2{53.5f, 31.5f});
        Prefabs::coin       (Vec2{51.5f, 19.5f});
        Prefabs::coin       (Vec2{51.5f, 21.5f});
        Prefabs::coin       (Vec2{58.5f, 19.5f});
        Prefabs::coin       (Vec2{59.5f, 20.5f});
        Prefabs::coin       (Vec2{45.5f, 38.5f});
        Prefabs::coin       (Vec2{48.5f, 37.5f});
        Prefabs::coin       (Vec2{45.5f, 51.5f});
        Prefabs::coin       (Vec2{49.5f, 54.5f});
        Prefabs::coin       (Vec2{56.5f, 57.5f});
        Prefabs::coin       (Vec2{60.5f, 57.5f});
        Prefabs::coin       (Vec2{56.5f, 59.5f});
        Prefabs::coin       (Vec2{60.5f, 59.5f});
        Prefabs::potion     (Vec2{4.5f, 8.5f});
        Prefabs::potion     (Vec2{10.5f, 13.5f});
        Prefabs::potion     (Vec2{8.5f, 20.5f});
        Prefabs::potion     (Vec2{5.5f, 33.5f});
        Prefabs::potion     (Vec2{28.5f, 31.5f});
        Prefabs::potion     (Vec2{47.5f, 30.5f});
        Prefabs::potion     (Vec2{56.5f, 58.5f});
        Prefabs::armor      (Vec2{11.5f, 4.5f});
        Prefabs::armor      (Vec2{20.5f, 21.5f});
        Prefabs::armor      (Vec2{51.5f, 28.5f});
        Prefabs::weapon_item(Vec2{17.5f, 21.5f}, Game::WEAPON_SMG);
        Prefabs::weapon_item(Vec2{41.5f, 30.5f}, Game::WEAPON_SHOTGUN);
        Prefabs::chest      (Vec2{12.5f, 32.5f}, 10, true, true);
        Prefabs::chest      (Vec2{60.5f, 18.5f}, 8, true, false);
        Prefabs::chest      (Vec2{58.5f, 60.5f}, 15, true, true);
        Prefabs::skeleton   (Vec2{5.5f, 5.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{12.5f, 3.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{14.5f, 5.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{11.5f, 10.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{13.5f, 12.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{8.5f, 14.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{10.5f, 15.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{3.5f, 13.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{4.5f, 18.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{3.5f, 22.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{4.5f, 26.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{8.5f, 19.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{10.5f, 21.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{15.5f, 18.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{20.5f, 18.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{17.5f, 20.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{19.5f, 22.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{3.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{6.5f, 33.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{5.5f, 32.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{10.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{13.5f, 33.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{24.5f, 29.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{28.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{26.5f, 30.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{17.5f, 26.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{17.5f, 28.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{34.5f, 29.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{36.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{41.5f, 29.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{43.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{48.5f, 28.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{53.5f, 28.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{50.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{53.5f, 31.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{51.5f, 19.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{53.5f, 21.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{58.5f, 19.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{46.5f, 37.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{48.5f, 39.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{48.5f, 44.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{48.5f, 46.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{45.5f, 51.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{49.5f, 51.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{45.5f, 54.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{49.5f, 54.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{56.5f, 57.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{60.5f, 57.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{56.5f, 59.5f}, /*facing=*/0.0f);
        Prefabs::skeleton   (Vec2{60.5f, 59.5f}, /*facing=*/0.0f);
        Prefabs::torch      (Vec2{2.5f, 6.5f});
        Prefabs::torch      (Vec2{6.5f, 2.5f});
        Prefabs::torch      (Vec2{11.5f, 2.5f});
        Prefabs::torch      (Vec2{17.5f, 2.5f});
        Prefabs::torch      (Vec2{23.5f, 2.5f});
        Prefabs::torch      (Vec2{16.5f, 17.5f});
        Prefabs::torch      (Vec2{20.5f, 22.5f});
        Prefabs::torch      (Vec2{35.5f, 28.5f});
        Prefabs::torch      (Vec2{47.5f, 28.5f});
        Prefabs::torch      (Vec2{50.5f, 18.5f});
        Prefabs::torch      (Vec2{45.5f, 50.5f});
        Prefabs::torch      (Vec2{60.5f, 56.5f});

        Prefabs::goal(Game::level_goal_pos);

        constexpr int W = 64;
        Game::DOOR_MODE[30 * W + 31] = Game::DOOR_MANUAL;
        Game::DOOR_LOCK[30 * W + 31] = 1;
    }

protected:
    bool tick(float dt) override {
        // Back: double-tap exits, single tap toggles inventory.
        if (input.was_double_tapped(InputKeyBack)) return false;
        if (input.was_pressed(InputKeyBack))       Game::inventory_open = !Game::inventory_open;

        // Level-complete state: OK restarts. All other gameplay frozen.
        if (Game::level_complete) {
            if (input.was_pressed(InputKeyOk)) setup_level();
            return true;
        }

        // Inventory open → input goes to the menu, gameplay freezes.
        if (Game::inventory_open) {
            sys_inventory_input(input);
            return true;
        }

        // Movement + raycast.
        Engine::Raycast::move_camera(input, camera, dt);
        Engine::Raycast::cast_rays_compute(camera);

        // Firing (handles cooldown, reload, semi vs auto, multi-pellet).
        sys_weapon_fire(camera, input, dt);

        sys_enemy_ai(camera, dt);
        sys_magnetism(camera);
        sys_motion_slide(dt);
        sys_motion_bounce(dt);    // engine bounce primitive — no current users
        sys_motion_bullet(dt);
        sys_anim(dt);
        sys_bob(dt);              // weapon pickups float up & down
        sys_doors(camera, dt);    // auto-open doors near the player
        sys_enemy_facing(camera);
        sys_enemy_attack(camera, dt);
        sys_bullet_damage();
        sys_damage_flash(dt);
        sys_lifetime(dt);
        sys_pickups(camera);
        sys_weapon_pickups(camera);
        sys_key_pickups(camera);
        sys_level(camera, dt);    // timer + goal-reach check
        return true;
    }

    void draw(Flip2D& c) override {
        Canvas* canvas = c.raw_canvas();
        canvas_clear(canvas);
        bmp_.clear();
        Engine::Raycast::draw_wall_strips(bmp_);
        sys_render_sprites(bmp_, camera);
        bmp_.commit(canvas);

        // Player-hit white flash — draws over the scene, under HUD/inventory.
        if (Game::player_damage_flash_timer > 0.0f) {
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_box(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        sys_render_healthbars(canvas, camera);
        sys_render_hud(canvas);
        sys_render_inventory(canvas);
        sys_render_stats(canvas);     // overlay only when level_complete
    }

private:
    inline static Bitmap1 bmp_;

    Engine::Raycast::Camera camera{
        Vec2{32.0f, 9.0f},   // center of north entry hall, between prison and library
        HALF_PI,             // facing south, toward the sewer/arena

        0.2f,
        3.0f,
        1.0f,
        PI * 0.5f,
        0.1f,
        20.0f,    // far clip — was 16; bigger 64x64 dungeon earns the extra reach
    };
};

extern "C" int32_t raycast_app(void* p) {
    UNUSED(p);
    RaycastApp app;
    return app.run();
}
