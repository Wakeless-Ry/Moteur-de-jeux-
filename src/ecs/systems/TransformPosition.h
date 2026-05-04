#ifndef TRANSFORM_POSITION
#define TRANSFORM_POSITION

#include <glm/ext.hpp>

#include "glm/detail/type_vec.hpp"
#include "src/GlobalScene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/systems/SystemUpdater.h"
#include "src/ecs/utils.h"

class TransformPosition : public UpdatableSystem {
    GlobalScene &scene;

  public:
    TransformPosition(GlobalScene &scene) : scene(scene) {}

    void update(float deltaTime) override {
        ECSManager &ecs = ECSManager::getManager();

        for (EntityId entity : this->getEntities()) {
            glm::vec3 position = ecs.getComponentOfEntity<Positionable>(entity)
                                     .value()
                                     .get()
                                     .pos;
            NodeId nodeId =
                ecs.getComponentOfEntity<Noded>(entity).value().get().id;

            this->scene.setTransform(nodeId, translate(position));
        }
    }

    void registerComponents(SystemId id) override {
        ECSManager::getManager().registerComponentToSystem<Positionable>(id);
        ECSManager::getManager().registerComponentToSystem<Noded>(id);
    }

    std::optional<glm::vec3> getPosition(EntityId entityId) {
        NodeId nodeId = ECSManager::getManager()
                            .getComponentOfEntity<Noded>(entityId)
                            .value()
                            .get()
                            .id;

        return this->scene.getTransform(nodeId).value().getPosition();
    }
};

#endif // TRANSFORM_POSITION