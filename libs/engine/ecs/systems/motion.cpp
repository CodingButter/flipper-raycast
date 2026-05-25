#include "systems.h"
#include "../components/transform.h"
#include "../components/velocity.h"

// Apply velocity to position. Pure data transform — no collision, no
// gravity. (Wall collisions and vertical physics belong to the raycast
// subsystem's sys_motion_slide / sys_motion_bounce.)
void sys_motion(float dt) {
    ECS::each(ECS::CMP_TRANSFORM | ECS::CMP_VELOCITY, [dt](ECS::EntityId e) {
        g_transforms[e].pos += g_velocities[e].vel * dt;
        g_transforms[e].z   += g_velocities[e].vz  * dt;
    });
}
