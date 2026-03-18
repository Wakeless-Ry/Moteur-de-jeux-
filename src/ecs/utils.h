#ifndef ECS_IDS
#define ECS_IDS

#include <bitset>
#include <functional>

#include "Component.h"

struct EntityId {
    unsigned long value;
    bool operator<(const EntityId &other) const { return value < other.value; }
    bool operator==(const EntityId &other) const {
        return value == other.value;
    }
};

struct SystemId {
    unsigned long value;
    bool operator<(const SystemId &other) const { return value < other.value; }
    bool operator==(const SystemId &other) const {
        return value == other.value;
    }
};

namespace std {
template <> struct hash<EntityId> {
    size_t operator()(const EntityId &id) const noexcept {
        return std::hash<unsigned long>{}(id.value);
    }
};
template <> struct hash<SystemId> {
    size_t operator()(const SystemId &id) const noexcept {
        return std::hash<unsigned long>{}(id.value);
    }
};
} // namespace std

namespace ECS {
const unsigned long MAX_ENTITIES = 8192;
using Signature = std::bitset<std::tuple_size_v<ComponentTypes>>;
} // namespace ECS

#endif // ECS_IDS