// Raycast-subsystem components — motion-behavior tags consumed by the
// raycast motion systems. Pull these in via this umbrella for the full set.
//
// Engine raycast bits (within the engine's 0-15 reserved range):
//   1u << 4   CMP_SLIDE    axis-separated wall slide
//   1u << 5   CMP_BOUNCE   axis-separated wall reflect (with damping)
//   1u << 6 .. 1u << 15   reserved for future raycast components
#pragma once

#include "slide.h"
#include "bounce.h"
