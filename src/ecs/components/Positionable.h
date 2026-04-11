#ifndef POSITIONABLE
#define POSITIONABLE

#include "glm/detail/type_vec.hpp"
#include <glm/ext.hpp>

struct Positionable {
    glm::vec3 pos = {0.7, 10, -1.5};

    Positionable() {}
    Positionable(glm::vec3 pos) : pos(pos) {}
};

#endif // POSITIONABLE