#ifndef CONTINUOUSLOD
#define CONTINUOUSLOD

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"

#include "src/Scene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/LOD.h"
#include "src/ecs/components/Positionable.h"
#include <iostream>
using uint = unsigned int;

class ContinuousLOD : public System {
    Scene &scene;

  public:
    ContinuousLOD(Scene &scene) : scene(scene) {}

    void update(float deltaTime, const glm::vec3 &cameraPos) {
        ECSManager &ecs = ECSManager::getManager();

        for (EntityId entity : this->getEntities()) {
            auto lodOpt = ecs.getComponentOfEntity<LOD>(entity);
            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);

            if (lodOpt.has_value() && posOpt.has_value()) {
                LOD &lod = lodOpt.value();
                Positionable &pos = posOpt.value();

                float distance = glm::length(pos.pos - cameraPos);

                this->simplifyMesh(lod, distance);
            }
        }
    }

  private:
    void simplifyMesh(LOD &lod, float distance) {
        if (abs(lod.current_distance - distance) > 10) {
            lod.current_distance = distance;
            lod.applyLOD(distance / 100);
            this->scene.getMesh(lod.id).value()->updateMeshData(
                lod.current_vertices, lod.current_indices, lod.current_normals,
                lod.current_uvs);
        }
    }
};

#endif