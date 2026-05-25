#include "world_api.h"

#include <math.h>

namespace Engine { namespace Raycast {

bool is_wall(const Vec2& grid_pos) {
    int x = (int)grid_pos.x;
    int y = (int)grid_pos.y;
    uint8_t c = cell_at(x, y);
    if (c == 0) return false;
    if (is_door(c)) {
        // Door blocks collision until it's mostly open.
        if (!DOOR_OPENNESS) return true;
        return DOOR_OPENNESS[y * WORLD_COLS + x] < DOOR_PASSABLE_THRESHOLD;
    }
    return true;
}

bool collides_with_wall(const Vec2& pos, float radius) {
    return is_wall(pos + Vec2{-radius, -radius})
        || is_wall(pos + Vec2{ radius, -radius})
        || is_wall(pos + Vec2{-radius,  radius})
        || is_wall(pos + Vec2{ radius,  radius});
}

bool has_line_of_sight(const Vec2& a, const Vec2& b) {
    Vec2  delta = b - a;
    float dist  = sqrtf(delta.sqr_length());
    if (dist < 0.001f) return true;
    // 8 samples per grid cell — fine grain enough that a wall corner
    // can't slip between two samples on a typical path.
    int   steps = (int)(dist * 8.0f) + 1;
    Vec2  step  = delta * (1.0f / (float)steps);
    Vec2  p     = a;
    // Skip the first sample (== a, assumed empty) and the last (== b).
    // Open doors are see-through — must match the visual rendering and
    // the movement collision (both treat openness ≥ 0.7 as passable).
    for (int i = 1; i < steps; i++) {
        p += step;
        int cx = (int)p.x;
        int cy = (int)p.y;
        uint8_t c = cell_at(cx, cy);
        if (c == 0) continue;
        if (is_door(c)) {
            uint8_t open = (DOOR_OPENNESS != nullptr)
                         ? DOOR_OPENNESS[cy * WORLD_COLS + cx]
                         : 0;
            if (open >= DOOR_PASSABLE_THRESHOLD) continue;
            return false;
        }
        return false;                      // solid wall
    }
    return true;
}

}} // namespace Engine::Raycast
