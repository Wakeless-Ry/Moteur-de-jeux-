
#ifndef PHYSICS
#define PHYSICS

#include <iostream>
#include <optional>

#include "glm/detail/type_vec.hpp"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/RigidBody.h"
#include "src/ecs/utils.h"

const glm::vec3 GRAVITY(0., -9.81, 0.);

class Physics : public System {
  public:
    void update(float deltaTime) {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {

            auto pos = ecs.getComponentOfEntity<Positionable>(entity);
            auto rigidBody = ecs.getComponentOfEntity<RigidBody>(entity);

            if (pos.has_value() && rigidBody.has_value()) {

                rigidBody.value().get().force += GRAVITY * deltaTime;
                rigidBody.value().get().velocity +=
                    rigidBody.value().get().force;
                pos.value().get().pos +=
                    rigidBody.value().get().velocity * deltaTime;
                rigidBody.value().get().force = glm::vec3(0);
            }
        }
    }
};

#endif