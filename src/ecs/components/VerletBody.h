#ifndef VERLET_BODY
#define VERLET_BODY

#include <glm/ext.hpp>

struct VerletBody {
    glm::vec3 last_position;
    glm::vec3 acceleration;
    float size;

    VerletBody() {}
    VerletBody(glm::vec3 last_position, glm::vec3 acceleration, float size)
        : last_position(last_position), acceleration(acceleration), size(size) {
    }
};

#endif // VERLET_BODY