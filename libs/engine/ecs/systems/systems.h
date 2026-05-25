// Engine ECS systems. Generic operations on the engine's core components
// — every game gets these for free. Renderer-tied systems (anything that
// touches Bitmap1 or Camera) live in libs/engine/<subsystem>/systems/.
//
// Each system is declared here and lives in its own .cpp.
#pragma once

void sys_motion(float dt);     // CMP_TRANSFORM + CMP_VELOCITY → applies velocity
void sys_anim(float dt);       // CMP_ANIM_SPRITE             → advances frame
void sys_lifetime(float dt);   // CMP_LIFETIME                → destroys at 0
