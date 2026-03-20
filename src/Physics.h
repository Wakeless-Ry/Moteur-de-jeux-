
#ifndef PHYSICS
#define PHYSICS

#include <iostream>
#include <optional>

#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/Velocity.h"
#include "src/ecs/utils.h"

const glm::vec3 GRAVITY(0., -9.81, 0.);

class Physics : public System {
  public:
    void update(float deltaTime) {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {
            auto pos = ecs.getComponentOfEntity<Positionable>(entity);
            auto velocity = ecs.getComponentOfEntity<Velocity>(entity);
            if (pos.has_value() && velocity.has_value()) {
                velocity.value().get().velocity += GRAVITY * deltaTime; 
                pos.value().get().pos +=
                    velocity.value().get().velocity * deltaTime;
                std::cout << pos.value().get().pos.y << std::endl;
            }
        }
    }
};

#endif