// Persistable raycast state — snapshots Camera + WORLD buffer to a file
// the game can store between runs.
//
// What this version handles:
//   • camera position + rotation
//   • world cell data (so runtime mutations to MAP_DATA persist)
//
// What it does NOT handle (yet):
//   • ECS entities + components — requires stable asset IDs to serialize
//     sprite-sheet pointers; see the comment at the bottom of state.cpp
//     for the design sketch.
//
// Usage from the game:
//   if (Engine::Raycast::load_state(SAVE_PATH, camera)) { … }
//   else { /* fresh start */ }
//   ...
//   Engine::Raycast::save_state(SAVE_PATH, camera);
#pragma once

#include "../vector.h"

namespace Engine { namespace Raycast {

struct Camera;   // forward decl — full def in camera.h

// File layout written/read by save_state / load_state:
//
//   uint32_t magic            // 'RYCT' (0x52594354)
//   uint32_t version          // bumped when layout changes; older saves rejected
//   Vec2     cam_pos
//   float    cam_rotation
//   uint8_t  world_cols
//   uint8_t  world_rows
//   uint8_t  world_data[cols * rows]
//
// load_state validates magic/version AND that the saved world dimensions
// match the current WORLD_COLS / WORLD_ROWS — a mismatch is a hard reject.
bool save_state(const char* path, const Camera& camera);
bool load_state(const char* path,       Camera& camera);

}} // namespace Engine::Raycast
