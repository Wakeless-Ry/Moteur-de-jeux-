
#ifndef GRAVITY
#define GRAVITY

#include "glm/detail/type_vec.hpp"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/systems/SystemUpdater.h"
#include "src/ecs/systems/TransformPosition.h"
#include <memory>

class Gravity : public UpdatableSystem {
    std::shared_ptr<TransformPosition> transformPosition;

  public:
    Gravity(std::shared_ptr<TransformPosition> transformPosition)
        : transformPosition(transformPosition) {}

    void update(float deltaTime) override {
        static const float GRAVITATIONAL_CONSTANT = 1;
        ECSManager &ecs = ECSManager::getManager();
        const std::set<EntityId> entities = this->getEntities();

        for (EntityId entity : entities) {
            glm::vec3 pos = ecs.getComponentOfEntity<Positionable>(entity)
                                .value()
                                .get()
                                .pos;
            VerletBody &verletBody =
                ecs.getComponentOfEntity<VerletBody>(entity).value();
            Attracted &attracted =
                ecs.getComponentOfEntity<Attracted>(entity).value();

            for (Attraction const &attraction : attracted.getAttractions()) {

                glm::vec3 bodyPos =
                    this->transformPosition->getPosition(attraction.body)
                        .value();

                float distance = glm::distance(pos, bodyPos);
                glm::vec3 direction = glm::normalize(bodyPos - pos);
                glm::vec3 acceleration =
                    direction * GRAVITATIONAL_CONSTANT *
                    (attraction.force / (distance * distance));

                verletBody.acceleration +=
                    acceleration * (attraction.mode == INWARD ? 1 : -1);
            }
        }
    }

    void registerComponents(SystemId id) override {
        ECSManager::getManager().registerComponentToSystem<Positionable>(id);
        ECSManager::getManager().registerComponentToSystem<VerletBody>(id);
        ECSManager::getManager().registerComponentToSystem<Attracted>(id);
    }
};

#endif // GRAVITY