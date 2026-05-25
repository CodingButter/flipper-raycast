// ECS — fixed-size entity/component framework for embedded.
//
// Design choices (deliberate, for the Flipper's 256KB-RAM tier):
//   • No allocations per frame. MAX_ENTITIES fixed at compile time.
//   • Components are bits in a u32 mask per entity. Add/has/remove are
//     single bitwise ops.
//   • Component data storage lives outside this framework (per-game in
//     game/components.cpp): one array of T per component, indexed by
//     EntityId. ECS just tracks which entities are alive and which
//     components they have.
//   • Iteration is a templated `each(mask, fn)` over a tiny linear scan.
//     At 64 entities that's 64 mask-checks per query — negligible.
//   • Adding a component type costs one new bit constant (in components.h)
//     and one new array (in components.cpp).
#pragma once

#include <stdint.h>

namespace ECS {

constexpr int MAX_ENTITIES = 64;

using EntityId      = int;
using ComponentMask = uint32_t;
constexpr EntityId NULL_ENTITY = -1;

// Internal storage exposed for template each<>(). Not for direct use.
namespace internal {
    extern bool          alive[MAX_ENTITIES];
    extern ComponentMask masks[MAX_ENTITIES];
}

// Lifecycle.
EntityId create();                                // returns NULL_ENTITY if full
void     destroy(EntityId e);
void     reset();                                 // kill all entities

// Queries.
bool          is_alive(EntityId e);
ComponentMask mask(EntityId e);
bool          has(EntityId e, ComponentMask required);

// Component-flag mutation.
void add(EntityId e, ComponentMask cm);
void remove_components(EntityId e, ComponentMask cm);

// Iterate live entities whose mask contains `required`.
//   ECS::each(CMP_TRANSFORM | CMP_VELOCITY, [dt](EntityId e) {
//       g_transforms[e].pos += g_velocities[e].vel * dt;
//   });
template<typename F>
inline void each(ComponentMask required, F&& fn) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (internal::alive[i] && (internal::masks[i] & required) == required) {
            fn((EntityId)i);
        }
    }
}

} // namespace ECS
