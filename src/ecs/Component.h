#ifndef COMPONENT
#define COMPONENT

#include <tuple>
#include <variant>

#include <glm/ext.hpp>

#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/RigidBody.h"
#include "src/ecs/components/VerletBody.h"

using ComponentTypes = std::tuple<Positionable, RigidBody, VerletBody>;

template <typename Tuple> struct VariantFromTuple;

template <typename... Ts> struct VariantFromTuple<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

using Component = VariantFromTuple<ComponentTypes>::type;

template <typename T, typename Tuple> struct is_in_tuple;

template <typename T> struct is_in_tuple<T, std::tuple<>> : std::false_type {};

template <typename T, typename First, typename... Rest>
struct is_in_tuple<T, std::tuple<First, Rest...>>
    : std::conditional_t<std::is_same_v<T, First>, std::true_type,
                         is_in_tuple<T, std::tuple<Rest...>>> {};

#endif // COMPONENT