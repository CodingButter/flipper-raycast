// Engine systems specific to the raycast subsystem — anything that needs
// the Camera, Bitmap1, world_api, or g_ray_hits to do its job.
//
// Pair each motion-behavior tag with its matching system. An entity should
// carry exactly one motion-behavior tag (SLIDE / BOUNCE / none) so it gets
// moved by exactly one system per frame.
#pragma once

#include "../../bitmap.h"
#include "../camera.h"

void sys_render_sprites(Bitmap1& b, const Engine::Raycast::Camera& cam);

void sys_motion_slide(float dt);    // CMP_SLIDE   — slide along walls
void sys_motion_bounce(float dt);   // CMP_BOUNCE  — reflect + damping + friction
