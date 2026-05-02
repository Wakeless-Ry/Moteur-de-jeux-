#ifndef VERLET_BODY
#define VERLET_BODY

#include <glm/ext.hpp>

struct VerletBody {
    glm::vec3 last_position;
    glm::vec3 acceleration;
    float size;
    float mass;
    bool unmovable;

    VerletBody() {}

    VerletBody(glm::vec3 last_position, glm::vec3 acceleration, float size,
               float mass, bool unmovable)
        : last_position(last_position), acceleration(acceleration), size(size),
          mass(mass), unmovable(unmovable) {}

    VerletBody(glm::vec3 last_position, glm::vec3 acceleration, float size,
               float mass)
        : VerletBody(last_position, acceleration, size, mass, false) {}
};

#endif // VERLET_BODY