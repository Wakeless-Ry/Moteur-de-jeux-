
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

#include "src/Terrain.cpp"

const glm::vec3 DOWN(0., -1, 0.);
const glm::vec3 UP(0., 1, 0.);
const float GRAVITY = 9.81;

class Physics : public System {
  private:
    std::optional<Terrain> terrain;

  public:
    void generateTerrain() { this->terrain = Terrain(); }
    void update(float deltaTime) {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {

            auto pos = ecs.getComponentOfEntity<Positionable>(entity);
            auto rigidBody = ecs.getComponentOfEntity<RigidBody>(entity);
            float y;
            bool setY = false;

            if (pos.has_value() && rigidBody.has_value()) {
                auto pair =
                    this->terrain.value().getContact(pos.value().get().pos);

                if (pair.has_value()) {

                    auto &[hauteur, normal] = pair.value();
                    if (pos.value().get().pos.y <= 0.2f + hauteur) {

                        rigidBody.value().get().force +=
                            rigidBody.value().get().mass * GRAVITY * normal *
                            deltaTime;

                        y = hauteur + 0.2f;
                        setY = true;
                    }
                }

                rigidBody.value().get().force +=
                    rigidBody.value().get().mass * GRAVITY * DOWN * deltaTime;

                float coef = 1.0f;
                if (pos.value().get().pos.y - 0.2f < 0.0f)
                    coef -= (100.0f * (pos.value().get().pos.y - 0.2f)) / 0.4f;
                float density_air = 1.0f;
                float density_eau = 1000.0f;

                float flotaison_air = std::clamp(density_air * GRAVITY * rigidBody.value().get().volume, 0.0f, GRAVITY + 2.0f);
                float flotaison_eau = std::clamp(density_air * GRAVITY * rigidBody.value().get().volume, 0.0f, GRAVITY + 2.0f);
                rigidBody.value().get().force += (coef * flotaison_air + (1 - coef) * flotaison_eau) * UP * deltaTime;
                printf("%f\n", coef);
                float traine;
                // if (rigidBody.value().get().velocity.x > 5)
                // {

                // }
                // else

                rigidBody.value().get().velocity +=
                    rigidBody.value().get().force /
                    rigidBody.value().get().mass;

                pos.value().get().pos +=
                    rigidBody.value().get().velocity * deltaTime;
                rigidBody.value().get().force = glm::vec3(0);


                if (setY) {
                    pos.value().get().pos.y =
                        std::max(pos.value().get().pos.y, y);
                    rigidBody.value().get().velocity.y = 0;
                }
            }
        }
    }

    Terrain &getTerrain() { return this->terrain.value(); }
};

#endif