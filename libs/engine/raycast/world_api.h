// Raycast world API — the interface the game implements so the engine can
// query the level. The engine declares these symbols; the game defines
// them (typically in game/world.cpp) and the linker stitches them together.
//
// Layout:
//   WORLD      flat row-major grid (WORLD_ROWS × WORLD_COLS bytes)
//              cell value 0 = empty, 1..N = wall type → index into WALL_TEXTURES
//   CELL_SIZE  pixels per cell for the top-down debug renderer
//
// This file also exposes the engine-side helpers (cell_at / is_wall /
// collides_with_wall) that work against whatever world the game provided.
#pragma once

#include <stdint.h>
#include "../vector.h"
#include "../assets/assets.h"   // Engine::Texture

namespace Engine { namespace Raycast {

// ===== Cell-type constants ============================================
//   0       empty (passable)
//   1, 2    solid wall types (game-specific textures)
//   3       sliding-panel door (game also provides DOOR_OPENNESS state)
//   4+      additional wall types (game-specific)
constexpr uint8_t DOOR_CELL_TYPE = 3;

inline bool is_door(uint8_t c) { return c == DOOR_CELL_TYPE; }

// ===== Game-provided symbols (defined by the game) ====================
// WORLD is non-const so Engine::Raycast::load_state can overwrite cells
// (e.g. for destructible walls or saved-state restoration). Normal engine
// code only reads through it via cell_at().

extern uint8_t* WORLD;
extern int      WORLD_COLS;
extern int      WORLD_ROWS;
extern int      CELL_SIZE;

// Lookup table of wall textures. Index 0 = sentinel (empty); 1..N = types.
extern const Engine::Texture* const* WALL_TEXTURES;
extern int                              WALL_TEXTURE_COUNT;

// Door state — openness per cell as 0..255 (uint8_t to save RAM on
// larger maps — at 64×64 this is 4KB instead of 16KB for floats).
// Read by the DDA + collision; written by the game's sys_doors. Cells
// that aren't doors are unused.
extern uint8_t* DOOR_OPENNESS;

// Threshold for "the door is open enough to pass / see through."
// Both movement collision and LOS use this; if you change it, change
// both. 178 ≈ 0.7 × 255.
constexpr uint8_t DOOR_PASSABLE_THRESHOLD = 178;

// ===== Engine helpers (operate on whatever the game provided) =========

// Inline so DDA's per-step cell read is a direct array index, not a call.
inline uint8_t cell_at(int x, int y) {
    if ((unsigned)x >= (unsigned)WORLD_COLS) return 1;  // OOB = solid
    if ((unsigned)y >= (unsigned)WORLD_ROWS) return 1;
    return WORLD[y * WORLD_COLS + x];
}

bool is_wall(const Vec2& grid_pos);
bool collides_with_wall(const Vec2& pos, float radius);

// Sampled line-of-sight: returns true if a straight line from `a` to `b`
// doesn't cross any wall cell. Sloppy DDA — 8 samples per cell, good
// enough for AI vision checks. Both endpoints are assumed to be in empty
// cells; behavior is undefined if they're inside walls.
bool has_line_of_sight(const Vec2& a, const Vec2& b);

}} // namespace Engine::Raycast
