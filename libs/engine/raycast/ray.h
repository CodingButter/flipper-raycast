// Ray casting — DDA grid traversal and per-column hit data.
//
// cast_rays_compute fills g_ray_hits[] on the app thread (in tick), so the
// GUI thread's render step is just data consumption — no math, no DDA work
// inside the draw callback.
#pragma once

#include "../vector.h"
#include "../flipper_app.h"   // SCREEN_WIDTH
#include "camera.h"

namespace Engine { namespace Raycast {

struct RayHit {
    Vec2  point;         // grid coord where the ray stopped
    float perp_dist;     // perpendicular distance from camera plane (avoids fisheye)
    float u;             // texture U coord in [0, 1) for the column we hit
    int   side;          // 0 = vertical wall (x-crossing), 1 = horizontal
    int   texture_id;    // 0 = no wall hit, else index into WALL_TEXTURES[]
};

// One RayHit per screen column. Written by cast_rays_compute (app thread),
// read by renderers (GUI thread).
extern RayHit g_ray_hits[SCREEN_WIDTH];

void cast_rays_compute(const Camera& camera);

}} // namespace Engine::Raycast
