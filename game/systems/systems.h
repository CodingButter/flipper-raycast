// Game system umbrella. Pulls in every engine-provided system AND every
// game-specific one, so callers (raycast.cpp) get them all from one
// include.
//
// Engine systems live in libs/engine/ecs/systems/ and libs/engine/raycast/systems/.
// Game-specific systems live next to this file.
#pragma once

// Engine-provided (sys_motion / sys_anim / sys_lifetime):
#include "../../libs/engine/ecs/systems/systems.h"
// Engine raycast (sys_render_sprites):
#include "../../libs/engine/raycast/systems/systems.h"
// Raycast Camera (forward-decl OK for Engine::Raycast::Camera below).
#include "../../libs/engine/raycast/camera.h"

// Forward declare Flipper SDK Canvas so this header doesn't pull in <gui/canvas.h>.
struct Canvas;

// === Game-specific systems ============================================
void sys_magnetism(const Engine::Raycast::Camera& cam); // pull collectibles toward player
void sys_pickups(const Engine::Raycast::Camera& cam);   // player vs collectibles
void sys_motion_bullet(float dt);                       // straight-line + destroy on wall
void sys_enemy_ai(const Engine::Raycast::Camera& cam, float dt);  // vision + walk toward player
void sys_enemy_facing(const Engine::Raycast::Camera& cam); // pick directional sheet + flip
void sys_enemy_attack(const Engine::Raycast::Camera& cam, float dt); // melee pulses
void sys_bullet_damage();                               // bullet vs CMP_HEALTH entities
void sys_damage_flash(float dt);                        // tick flash_timer → color_invert
void sys_render_healthbars(Canvas* canvas, const Engine::Raycast::Camera& cam);  // 10-chunk bars
void sys_render_hud(Canvas* canvas);                    // crosshair, counters, gun icon
void sys_render_inventory(Canvas* canvas);              // overlay when inventory_open

// Weapons + pickups (game-specific).
void sys_weapon_fire(const Engine::Raycast::Camera& cam, const Input& input, float dt);
void sys_weapon_pickups(const Engine::Raycast::Camera& cam);
void sys_key_pickups(const Engine::Raycast::Camera& cam);
void sys_bob(float dt);

// World — auto-open doors when the player is within range.
void sys_doors(const Engine::Raycast::Camera& cam, float dt);

// Level loop — timer + completion check + stats overlay.
void sys_level(const Engine::Raycast::Camera& cam, float dt);
void sys_render_stats(Canvas* canvas);

// Inventory navigation — call only when Game::inventory_open. Up/Down
// scroll between owned weapons, OK equips highlighted.
void sys_inventory_input(const Input& input);
