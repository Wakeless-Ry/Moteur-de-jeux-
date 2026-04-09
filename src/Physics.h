
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
const float V_total = (4.0f / 3.0f) * M_PI * r * r * r;
const float eta = 0.001f;
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
                    float V_total = (4.0f / 3.0f) * M_PI * r * r * r;
                    float density_balle = rigidBody.value().get().mass / V_total;
                    float coef = density_balle / density_eau;

                    float deep = waterLevel - pos.value().get().pos.y;
                    float height = deep + r;
                    height = std::clamp(height, 0.0f, 2.0f * r);

                    float V_immerge = M_PI * height * height * (3.0f * r - height) / 3.0f;

                    float Flotaison = density_eau * GRAVITY * V_immerge * coef;
                    rigidBody.value().get().force += Flotaison * 2.0f * UP;

                    glm::vec3 v = rigidBody.value().get().velocity;
                    float speed = glm::length(v);
                    if (speed > 0.001f)
                    {
                        float immersion = std::clamp((waterLevel - pos.value().get().pos.y) / (2.0f * r), 0.0f, 1.0f);
                        glm::vec3 dir = v / speed;

                        float Re = density_eau * speed * r / eta;
                        glm::vec3 dragWater;

                        const float eta = 0.001f;
                        if (Re < 1.0f) {
                            float b = 6.0f * M_PI * eta * r;
                            dragWater = -b * v;
                        } else {
                            dragWater = -0.5f * density_eau * Cd * A_ * speed * speed * dir;
                        }

                        dragWater *= immersion * coef;
                        rigidBody.value().get().force += dragWater;
                    }
                }

                if (true) {
                    glm::vec3 B = glm::vec3(0.0f, 15.0f, 0.0f);
                    float l = 2.0f;
                    float K = 10.0f;
                    float C = 1.0f;

                    glm::vec3 diff = pos.value().get().pos - B;
                    float dist = glm::length(diff);

                    glm::vec3 dir = diff / dist;
                    glm::vec3 Fk = -K * (dist - l) * dir;

                    glm::vec3 Fc = -C * rigidBody.value().get().velocity;

                    rigidBody.value().get().force += Fk + Fc;
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