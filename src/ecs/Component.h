#ifndef COMPONENT
#define COMPONENT

#include <tuple>

#include <glm/ext.hpp>
#include <variant>

#include "src/Transform.h"

struct Positionable {
    glm::vec3 pos;
};

struct Transformable {
    Transform t;
};

using ComponentTypes = std::tuple<Positionable, Transformable>;

template <typename Tuple> struct VariantFromTuple;

template <typename... Ts> struct VariantFromTuple<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

using Component = VariantFromTuple<ComponentTypes>::type;

#endif // COMPONENT