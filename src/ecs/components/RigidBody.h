#ifndef RIGIDBODY
#define RIGIDBODY

#include "glm/detail/type_vec.hpp"
#include <glm/ext.hpp>

struct RigidBody {
    glm::vec3 velocity;
    glm::vec3 force;
    float mass = 1.f;

    RigidBody() {}
    RigidBody(glm::vec3 force) : force(force) {}
};

#endif // RIGIDBODY