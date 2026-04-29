#ifndef TRANSFORM_POSITION
#define TRANSFORM_POSITION

#include <glm/ext.hpp>

#include "glm/detail/type_vec.hpp"
#include "src/GlobalScene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"

class TransformPosition : public System {
    GlobalScene &scene;

  public:
    TransformPosition(GlobalScene &scene) : scene(scene) {}

    void update(float deltaTime) {
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
};

#endif // TRANSFORM_POSITION