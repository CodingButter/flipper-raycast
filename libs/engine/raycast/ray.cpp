#include "ray.h"
#include "world_api.h"
#include "../constants.h"
#include <math.h>

namespace Engine { namespace Raycast {

RayHit g_ray_hits[SCREEN_WIDTH];

// DDA grid traversal with sliding-panel door support.
//
// For regular walls: stops at the cell entry boundary (standard DDA).
//
// For door cells: the door's plane sits at the cell's center along
// whichever axis the ray most recently crossed (Lode-style convention).
// We compute the ray's intersection with that plane, evaluate the door's
// `u` (texture column the ray would hit), and check it against the
// door's openness window `|u - 0.5| < open/2`. If inside the window →
// ray passes through the gap and DDA continues to the next cell. If
// outside → the ray hits a panel; we return that as the hit.
static RayHit dda_cast(const Vec2& origin, const Vec2& dir, float max_dist) {
    int map_x = (int)origin.x;
    int map_y = (int)origin.y;
    float delta_dx = (dir.x == 0.0f) ? 1.0e30f : fabsf(1.0f / dir.x);
    float delta_dy = (dir.y == 0.0f) ? 1.0e30f : fabsf(1.0f / dir.y);
    int   step_x = (dir.x < 0) ? -1 : 1;
    int   step_y = (dir.y < 0) ? -1 : 1;
    float side_dx = (dir.x < 0) ? (origin.x - map_x) * delta_dx
                                : (map_x + 1.0f - origin.x) * delta_dx;
    float side_dy = (dir.y < 0) ? (origin.y - map_y) * delta_dy
                                : (map_y + 1.0f - origin.y) * delta_dy;

    float cur_dist = 0.0f;
    int   side     = 0;
    int   tex_id   = 0;
    Vec2  hit_point = origin;

    while (true) {
        if (side_dx < side_dy) {
            cur_dist = side_dx;
            side_dx += delta_dx;
            map_x   += step_x;
            side = 0;
        } else {
            cur_dist = side_dy;
            side_dy += delta_dy;
            map_y   += step_y;
            side = 1;
        }
        if ((unsigned)map_x >= (unsigned)WORLD_COLS) break;
        if ((unsigned)map_y >= (unsigned)WORLD_ROWS) break;

        uint8_t c = WORLD[map_y * WORLD_COLS + map_x];
        if (c == 0) {
            if (cur_dist > max_dist) break;
            continue;
        }

        // Door cell — check the center-plane intersection.
        if (is_door(c)) {
            float door_t;
            Vec2  door_hit;
            if (side == 0) {
                float door_x = (float)map_x + 0.5f;
                door_t = (door_x - origin.x) / dir.x;
                door_hit = Vec2{door_x, origin.y + dir.y * door_t};
            } else {
                float door_y = (float)map_y + 0.5f;
                door_t = (door_y - origin.y) / dir.y;
                door_hit = Vec2{origin.x + dir.x * door_t, door_y};
            }
            if (door_t > max_dist) break;

            float u_on_door = (side == 0) ? (door_hit.y - floorf(door_hit.y))
                                          : (door_hit.x - floorf(door_hit.x));
            // DOOR_OPENNESS is uint8 (0..255). Convert to float once per
            // door hit for the gap math.
            float open = (DOOR_OPENNESS != nullptr)
                       ? (float)DOOR_OPENNESS[map_y * WORLD_COLS + map_x] * (1.0f / 255.0f)
                       : 0.0f;
            float gap_half = open * 0.5f;

            if (fabsf(u_on_door - 0.5f) >= gap_half) {
                // Hit a door panel.
                tex_id    = c;
                hit_point = door_hit;
                cur_dist  = door_t;
                break;
            }
            // Ray passes through the gap — let DDA advance to the next cell.
            if (cur_dist > max_dist) break;
            continue;
        }

        // Regular wall hit.
        tex_id    = c;
        hit_point = Vec2{origin.x + dir.x * cur_dist, origin.y + dir.y * cur_dist};
        break;
    }

    // No hit → final point at cur_dist for cast_rays_compute to project.
    if (tex_id == 0) {
        hit_point = Vec2{origin.x + dir.x * cur_dist, origin.y + dir.y * cur_dist};
    }

    RayHit r{};
    r.point      = hit_point;
    r.side       = side;
    r.texture_id = tex_id;
    return r;
}

void cast_rays_compute(const Camera& camera) {
    const Vec2  fwd{cosf(camera.rotation), sinf(camera.rotation)};
    const Vec2  plane_step = (camera.pr - camera.pl) * (1.0f / (float)(SCREEN_WIDTH - 1));
    Vec2        target     = camera.pl;
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        Vec2 dir = target - camera.position;
        dir.normalize();
        RayHit h = dda_cast(camera.position, dir, camera.far_clip);

        Vec2 off    = h.point - camera.position;
        h.perp_dist = off.x * fwd.x + off.y * fwd.y;

        h.u = (h.side == 0) ? (h.point.y - floorf(h.point.y))
                            : (h.point.x - floorf(h.point.x));

        g_ray_hits[i] = h;
        target += plane_step;
    }
}

}} // namespace Engine::Raycast
