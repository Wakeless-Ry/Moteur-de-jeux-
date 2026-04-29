
#ifndef PHYSICS
#define PHYSICS

#include <optional>

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/RigidBody.h"
#include "src/ecs/utils.h"

#include "src/Terrain.cpp"

const glm::vec3 DOWN(0., -1, 0.);
const float GRAVITY = 9.81;
const float STATIC_FRICTION = 0.74;
const float KINETIC_FRICTION = 0.57;

class Physics : public System {
  private:
    std::optional<Terrain> terrain;

  public:
    void generateTerrain() { this->terrain = Terrain(); }
    void update(float deltaTime) override {
        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {

            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);
            auto rigidBodyOpt = ecs.getComponentOfEntity<RigidBody>(entity);
            float y;
            bool touchingGround = false;

            if (posOpt.has_value() && rigidBodyOpt.has_value()) {
                Positionable &pos = posOpt.value();
                RigidBody &rigidBody = rigidBodyOpt.value();

                glm::vec3 weight = rigidBody.mass * GRAVITY * DOWN;
                glm::vec3 projection;
                auto pair = this->terrain.value().getProjectedContact(pos.pos);

                if (pair.has_value()) {
                    auto &[hauteur, normal] = pair.value();
                    touchingGround = pos.pos.y <= 0.2f + hauteur;

                    if (touchingGround) {
                        float dotProduct = glm::dot(-weight, normal);
                        float square = glm::dot(normal, normal);
                        projection = (dotProduct / square) * normal;

                        rigidBody.force += projection * deltaTime;

                        y = hauteur + 0.2f;
                    }
                }

                rigidBody.force += weight * deltaTime;
                rigidBody.velocity += rigidBody.force / rigidBody.mass;

                glm::vec3 horizontalVelocity = rigidBody.velocity;
                horizontalVelocity.y = 0;

                if (touchingGround) {
                    if (glm::length(horizontalVelocity) <=
                        glm::length(projection) * STATIC_FRICTION * deltaTime) {

                        rigidBody.velocity -= horizontalVelocity;
                    } else {
                        rigidBody.velocity *= 0.98;
                    }
                }

                pos.pos += rigidBody.velocity * deltaTime;
                rigidBody.force = glm::vec3(0);

                if (touchingGround) {
                    pos.pos.y = std::max(pos.pos.y, y);
                    rigidBody.velocity.y = 0;
                }
            }
        }
    }

    void registerComponents(SystemId id) override {
        ECSManager::getManager().registerComponentToSystem<Positionable>(id);
        ECSManager::getManager().registerComponentToSystem<RigidBody>(id);
    }

    Terrain &getTerrain() { return this->terrain.value(); }
};

#endif