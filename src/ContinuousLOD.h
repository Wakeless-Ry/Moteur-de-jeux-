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
        LODLevel newLevel = LOD::levelFromDistance(distance);

        if (newLevel == lod.current_level)
            return;

        lod.current_level = newLevel;

        float resolution;
        switch (newLevel) {
        case LODLevel::HIGH:
            resolution = 0.1f;
            break;
        case LODLevel::MEDIUM:
            resolution = 0.5f;
            break;
        case LODLevel::LOW:
            resolution = 2.0f;
            break;
        default:
            resolution = 1.0f;
            break;
        }

        lod.applyLOD(resolution);
        this->scene.getMesh(lod.id).value()->updateMeshData(
            lod.current_vertices, lod.current_indices, lod.current_normals,
            lod.current_uvs);
    }
};

#endif