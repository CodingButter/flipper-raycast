#include "systems.h"
#include "../state.h"
#include "../../libs/engine/raycast/world_api.h"
#include "../../libs/engine/raycast/camera.h"

namespace {
constexpr float OPEN_RANGE_SQ = 2.5f * 2.5f;
constexpr float OPEN_SPEED    = 2.5f;   // 0..1 per second (×255 in uint8 space)
}

// AUTO doors track player proximity; MANUAL doors are toggled by player
// interaction. Both kinds lerp DOOR_OPENNESS toward DOOR_TARGET in uint8
// space (255 = fully open).
void sys_doors(const Engine::Raycast::Camera& cam, float dt) {
    if (!Engine::Raycast::DOOR_OPENNESS) return;
    if (!Game::DOOR_TARGET || !Game::DOOR_MODE) return;

    using Engine::Raycast::WORLD;
    using Engine::Raycast::WORLD_COLS;
    using Engine::Raycast::WORLD_ROWS;
    using Engine::Raycast::DOOR_OPENNESS;

    // 2.5 * 255 / 30 fps ≈ 21 per tick → full open in ~12 ticks (~0.4s).
    int step = (int)(OPEN_SPEED * 255.0f * dt);
    if (step < 1) step = 1;

    for (int y = 0; y < WORLD_ROWS; y++) {
        for (int x = 0; x < WORLD_COLS; x++) {
            int idx = y * WORLD_COLS + x;
            uint8_t c = WORLD[idx];
            if (!Engine::Raycast::is_door(c)) continue;

            // Locked auto doors stay closed unless we hold the matching key.
            // Manual doors handle the lock check at toggle time instead.
            bool locked = false;
            if (Game::DOOR_LOCK && Game::DOOR_LOCK[idx] > 0) {
                int need = Game::DOOR_LOCK[idx] - 1;
                locked = (need >= Game::KEY_COUNT) || !Game::keys_owned[need];
            }

            if (Game::DOOR_MODE[idx] == Game::DOOR_AUTO) {
                float dx = cam.position.x - ((float)x + 0.5f);
                float dy = cam.position.y - ((float)y + 0.5f);
                bool near = (dx*dx + dy*dy < OPEN_RANGE_SQ);
                Game::DOOR_TARGET[idx] = (near && !locked) ? 255 : 0;
            }

            int target = Game::DOOR_TARGET[idx];
            int open   = DOOR_OPENNESS[idx];
            int diff   = target - open;
            if (diff > step)        open += step;
            else if (diff < -step)  open -= step;
            else                    open = target;
            DOOR_OPENNESS[idx] = (uint8_t)open;
        }
    }
}
