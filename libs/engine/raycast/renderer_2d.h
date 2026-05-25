// Top-down debug renderer. Useful for verifying world layout, camera
// position, and raycaster behavior. Each function draws into the bitmap;
// call them in any combination from your app's draw().
#pragma once

#include "../bitmap.h"
#include "../vector.h"
#include "camera.h"

namespace Engine { namespace Raycast {

// Camera-relative world→screen projection. Centers `camera.position` on
// screen and scales by CELL_SIZE.
Vec2 grid_to_screen(const Vec2& grid, const Camera& cam);

void draw_grid(Bitmap1& b, const Camera& camera);
void draw_walls(Bitmap1& b, const Camera& camera);
void draw_fov(Bitmap1& b, const Camera& camera);
void draw_camera_marker(Bitmap1& b, const Camera& camera);
void draw_ray_hits(Bitmap1& b, const Camera& camera);

}} // namespace Engine::Raycast
