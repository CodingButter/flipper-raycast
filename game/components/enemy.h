// Enemy — vision + path-following AI state in one component. Stays small
// by only caching the *next* step rather than a full path: we re-plan via
// A* whenever the enemy crosses into a new cell.
//
// Vision: range + cone half-angle + line-of-sight check (sys_enemy_ai
// runs all three each tick). Cone is measured from `facing`, so a
// cone_half of π/3 = a 120° total field of view.
#pragma once

#include "../../libs/engine/ecs/ecs.h"

namespace ECS { constexpr ComponentMask CMP_ENEMY = 1u << 21; }

struct Enemy {
    // --- Configuration -------------------------------------------
    float facing;             // radians; convention: 0 = +X, +π/2 = +Y
    float walk_speed;         // idle wander speed (cells/sec); 0 = stationary
    float chase_speed;        // speed while chasing a target (cells/sec)
    float vision_range;       // cells
    float vision_cone_half;   // radians (half-angle); π = sees in all directions

    // --- Idle wander timers --------------------------------------
    float turn_timer;
    float turn_period;
    float turn_angle;

    // --- AI / pathfinding state ----------------------------------
    bool  has_target;         // true = actively chasing a known cell
    int   target_cell_x;      // player's last known cell
    int   target_cell_y;
    int   step_cell_x;        // (vestigial — was A*; kept to avoid prefab churn)
    int   step_cell_y;
    int   last_cell_x;
    int   last_cell_y;
    bool  step_valid;

    // --- Melee attack --------------------------------------------
    float attack_timer;       // seconds until next melee attack is allowed
};

extern Enemy g_enemies[ECS::MAX_ENTITIES];
