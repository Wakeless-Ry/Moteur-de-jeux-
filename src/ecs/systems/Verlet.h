
#ifndef VERLET
#define VERLET

#include <optional>

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"

#include "src/ecs/ECSManager.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/SystemUpdater.h"
#include "src/ecs/utils.h"

class Verlet : public UpdatableSystem {
    const double GRAVITATIONAL_CONSTANT = .001;

  public:
    void update(float deltaTime) override {
        this->applyGravity();
        this->solveCollisions();
        this->updateBallPositions(deltaTime);
    }

    void registerComponents(SystemId id) override {
        ECSManager::getManager().registerComponentToSystem<Positionable>(id);
        ECSManager::getManager().registerComponentToSystem<VerletBody>(id);
    }

    void applyGravity() {
        ECSManager &ecs = ECSManager::getManager();
        const std::set<EntityId> entities = this->getEntities();

        for (EntityId first : entities) {
            glm::vec3 pos1 =
                ecs.getComponentOfEntity<Positionable>(first).value().get().pos;
            VerletBody &verletBody1 =
                ecs.getComponentOfEntity<VerletBody>(first).value();

            for (EntityId second : entities) {
                if (first != second) {
                    glm::vec3 pos2 =
                        ecs.getComponentOfEntity<Positionable>(second)
                            .value()
                            .get()
                            .pos;
                    VerletBody &verletBody2 =
                        ecs.getComponentOfEntity<VerletBody>(second).value();

                    if (verletBody2.mass != 0) {
                        float distance = glm::distance(pos1, pos2);
                        glm::vec3 direction = glm::normalize(pos2 - pos1);

                        glm::vec3 acceleration =
                            direction * GRAVITATIONAL_CONSTANT *
                            (verletBody2.mass / (distance * distance));

                        verletBody1.acceleration += acceleration;
                    }
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