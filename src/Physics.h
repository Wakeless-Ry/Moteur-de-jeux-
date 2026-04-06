
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
const float r = 0.2f;
const float waterLevel = 0.0f;
const float density_eau = 1000.0f;
const float density_air = 1.0f;
const float Cd = 0.47f;
const float A_ = M_PI * r * r;
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
                            rigidBody.value().get().mass * GRAVITY * normal;

                        y = hauteur + 0.2f;
                        setY = true;
                    }
                }

                rigidBody.value().get().force +=
                    rigidBody.value().get().mass * GRAVITY * DOWN;

                bool inWater = pos.value().get().pos.y < waterLevel;

                if (inWater) {
                    float deep = waterLevel - pos.value().get().pos.y;
                    float height = deep + r;
                    height = std::clamp(height, 0.0f, 2.0f * r);

                    float V_immerge = M_PI * height * height * (3 * r - height) / 3.0f;

                    float Flotaison = density_eau * GRAVITY * V_immerge * 0.05f;

                    rigidBody.value().get().force += Flotaison * UP;
                    glm::vec3 v = rigidBody.value().get().velocity;
                    glm::vec3 vHoriz(v.x, 0.0f, v.z);
                    float speed = glm::length(vHoriz);
                    if (speed > 0.001f)
                    {
                        glm::vec3 dragWater(0.0f);
                        glm::vec3 dir = vHoriz / speed;
                        dragWater = -0.5f * density_eau * Cd * A_ * speed * speed * dir;

                        float immersion = std::clamp((waterLevel - pos.value().get().pos.y) / (2.0f * r), 0.0f, 1.0f);
                        dragWater *= immersion * 0.0001f;
                        rigidBody.value().get().force += dragWater;
                    }
                }

                rigidBody.value().get().velocity +=
                    (rigidBody.value().get().force /
                    rigidBody.value().get().mass) *
                    deltaTime;

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