
#ifndef VERLET
#define VERLET

#include <optional>

#include "glm/detail/type_vec.hpp"

#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/utils.h"

class Verlet : public System {
    const glm::vec3 GRAVITY = {0, -9.81, 0};
    const float GRAVITATIONAL_CONSTANT = 10.0f;

    struct OrbitalBody {
        glm::vec3 position;
        float mass;
        bool enabled;
    };
    OrbitalBody orbitalBody = {{0, 30, 0}, 100.0f, false};

  public:
    void setOrbitalBody(glm::vec3 position, float mass) {
        orbitalBody.position = position;
        orbitalBody.mass = mass;
        orbitalBody.enabled = true;
    }
    glm::vec3 calculateOrbitalVelocity(glm::vec3 objectPos,
                                       glm::vec3 tangentDir) {
        glm::vec3 toCenter = orbitalBody.position - objectPos;
        float distance = glm::length(toCenter);

        if (distance < 0.001f)
            return glm::vec3(0);

        float orbitalSpeed =
            glm::sqrt(GRAVITATIONAL_CONSTANT * orbitalBody.mass / distance);
        return glm::normalize(tangentDir) * orbitalSpeed;
    }

    void update(float deltaTime) override {
        float radius = 42;

        this->applyGravity();
        this->applyFloorConstraint(radius);
        this->solveCollisions();
        this->updateBallPositions(deltaTime);
    }

    void registerComponents(SystemId id) override {
        ECSManager::getManager().registerComponentToSystem<Positionable>(id);
        ECSManager::getManager().registerComponentToSystem<VerletBody>(id);
    }

    void applyGravity() {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {
            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);
            auto verletBodyOpt = ecs.getComponentOfEntity<VerletBody>(entity);

            if (posOpt.has_value() && verletBodyOpt.has_value()) {
                Positionable &pos = posOpt.value();
                VerletBody &verletBody = verletBodyOpt.value();

                if (orbitalBody.enabled) {
                    glm::vec3 toCenter = orbitalBody.position - pos.pos;
                    float distanceSq = glm::length2(toCenter);

                    if (distanceSq > 0.001f) {
                        float distance = glm::sqrt(distanceSq);
                        glm::vec3 direction = toCenter / distance;
                        float acceleration = GRAVITATIONAL_CONSTANT *
                                             orbitalBody.mass / distanceSq;

                        verletBody.acceleration += direction * acceleration;
                    }
                } else {
                    verletBody.acceleration += GRAVITY;
                }
            }
        }
    }

    void applyFloorConstraint(float radius) {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {
            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);
            auto verletBodyOpt = ecs.getComponentOfEntity<VerletBody>(entity);

            if (posOpt.has_value() && verletBodyOpt.has_value()) {
                Positionable &pos = posOpt.value();
                VerletBody &verletBody = verletBodyOpt.value();

                if (pos.pos.x > radius) {
                    pos.pos.x = radius;
                }
                if (pos.pos.x < -radius) {
                    pos.pos.x = -radius;
                }

                if (pos.pos.z > radius) {
                    pos.pos.z = radius;
                }
                if (pos.pos.z < -radius) {
                    pos.pos.z = -radius;
                }
            }
        }
    }

    void solveCollisions() {
        ECSManager &ecs = ECSManager::getManager();
        const std::set<EntityId> entities = this->getEntities();

        for (auto it1 = entities.begin(); it1 != entities.end(); it1++) {
            EntityId id1 = *it1;
            auto posOpt1 = ecs.getComponentOfEntity<Positionable>(id1);
            auto verletBodyOpt1 = ecs.getComponentOfEntity<VerletBody>(id1);

            if (posOpt1.has_value() && verletBodyOpt1.has_value()) {
                Positionable &pos1 = posOpt1.value();
                VerletBody &verletBody1 = verletBodyOpt1.value();

                auto it2 = it1;
                it2++;
                for (; it2 != entities.end(); it2++) {
                    EntityId id2 = *it2;
                    auto posOpt2 = ecs.getComponentOfEntity<Positionable>(id2);
                    auto verletBodyOpt2 =
                        ecs.getComponentOfEntity<VerletBody>(id2);

                    if (posOpt2.has_value() && verletBodyOpt2.has_value()) {
                        Positionable &pos2 = posOpt2.value();
                        VerletBody &verletBody2 = verletBodyOpt2.value();

                        glm::vec3 collisionAxis = pos1.pos - pos2.pos;
                        float dist = glm::length(collisionAxis);

                        if (dist > 0.0001 &&
                            dist < verletBody1.size + verletBody2.size) {
                            glm::vec3 n = collisionAxis / dist;
                            float diff =
                                verletBody1.size + verletBody2.size - dist;

                            pos1.pos +=
                                (verletBody1.size /
                                 (verletBody1.size + verletBody2.size)) *
                                diff * n;
                            pos2.pos -=
                                (verletBody2.size /
                                 (verletBody1.size + verletBody2.size)) *
                                diff * n;
                        }
                    }
                }
            }
        }
    }

    void updateBallPositions(float deltaTime) {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {
            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);
            auto verletBodyOpt = ecs.getComponentOfEntity<VerletBody>(entity);

            if (posOpt.has_value() && verletBodyOpt.has_value()) {
                Positionable &pos = posOpt.value();
                VerletBody &verletBody = verletBodyOpt.value();
                glm::vec3 velocity = pos.pos - verletBody.last_position;
                verletBody.last_position = pos.pos;

                pos.pos +=
                    velocity + verletBody.acceleration * deltaTime * deltaTime;

                verletBody.acceleration = {};
            }
        }
    }
};

#endif