#ifndef COMPONENT
#define COMPONENT

#include <tuple>
#include <variant>

#include <glm/ext.hpp>

#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/RigidBody.h"

using ComponentTypes = std::tuple<Positionable, RigidBody>;

template <typename Tuple> struct VariantFromTuple;

template <typename... Ts> struct VariantFromTuple<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

using Component = VariantFromTuple<ComponentTypes>::type;

#endif // COMPONENT