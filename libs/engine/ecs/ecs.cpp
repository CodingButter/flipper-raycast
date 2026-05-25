#include "ecs.h"

namespace ECS {
namespace internal {
    bool          alive[MAX_ENTITIES] = {};
    ComponentMask masks[MAX_ENTITIES] = {};
}

EntityId create() {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!internal::alive[i]) {
            internal::alive[i] = true;
            internal::masks[i] = 0;
            return (EntityId)i;
        }
    }
    return NULL_ENTITY;
}

void destroy(EntityId e) {
    if ((unsigned)e >= (unsigned)MAX_ENTITIES) return;
    internal::alive[e] = false;
    internal::masks[e] = 0;
}

void reset() {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        internal::alive[i] = false;
        internal::masks[i] = 0;
    }
}

bool is_alive(EntityId e) {
    if ((unsigned)e >= (unsigned)MAX_ENTITIES) return false;
    return internal::alive[e];
}

ComponentMask mask(EntityId e) {
    if ((unsigned)e >= (unsigned)MAX_ENTITIES) return 0;
    return internal::masks[e];
}

bool has(EntityId e, ComponentMask required) {
    return (mask(e) & required) == required;
}

void add(EntityId e, ComponentMask cm) {
    if ((unsigned)e >= (unsigned)MAX_ENTITIES) return;
    internal::masks[e] |= cm;
}

void remove_components(EntityId e, ComponentMask cm) {
    if ((unsigned)e >= (unsigned)MAX_ENTITIES) return;
    internal::masks[e] &= ~cm;
}

} // namespace ECS
