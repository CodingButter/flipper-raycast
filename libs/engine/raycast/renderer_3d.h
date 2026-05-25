// First-person renderer — wall strips + billboard sprite primitive.
//
// Both routines read g_ray_hits (filled by cast_rays_compute on the app
// thread) and draw into a Bitmap1 the caller will commit to canvas.
//
// draw_billboard is a *primitive*: it draws ONE sprite. The game owns the
// loop / data layout / picking who to draw (typically via an ECS system).
#pragma once

#include "../bitmap.h"
#include "../assets/assets.h"  // Engine::SpriteSheet
#include "camera.h"

namespace Engine { namespace Raycast {

void draw_wall_strips(Bitmap1& b);

// Project a world point (`world_xy`, `z`) into screen pixel space using
// the same math sprite/wall rendering uses. Returns false if the point
// is behind the camera (depth ≤ 0); otherwise fills *out_sx, *out_sy
// (may be off-screen) and *out_depth (perpendicular distance in cells).
// Useful for HUD overlays — trajectory previews, debug markers, etc.
bool project(const Camera& cam, Vec2 world_xy, float z,
             int* out_sx, int* out_sy, float* out_depth);

// Renders one frame of a sprite sheet as a camera-facing billboard at
// (`world_pos`, height `z`). `scale` is a world-size multiplier (1.0 = 1
// grid unit tall). `flip_horizontal` mirrors U sampling — used by
// direction-aware sprites to render mirrored angles without storing a
// separate flipped copy. Per-column z-buffer test against g_ray_hits so
// walls correctly occlude. Pixel encoding: 0=transparent, 1=black, 2=white.
void draw_billboard(Bitmap1& b, const Camera& cam,
                    Vec2 world_pos, float z,
                    const Engine::SpriteSheet* sheet, int frame, float scale,
                    bool flip_horizontal, bool color_invert);

}} // namespace Engine::Raycast
