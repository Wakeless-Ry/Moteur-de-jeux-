#ifndef CONTINUOUSLOD
#define CONTINUOUSLOD

#include "glm/detail/func_geometric.hpp"

#include "src/Camera.h"
#include "src/GlobalScene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/LOD.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/systems/SystemUpdater.h"

using uint = unsigned int;

class ContinuousLOD : public UpdatableSystem {
    GlobalScene &scene;
    Camera &camera;

  public:
    ContinuousLOD(GlobalScene &scene, Camera &camera)
        : scene(scene), camera(camera) {}

    void registerComponents(SystemId id) override {
        ECSManager::registerComponentToSystem<Positionable>(id);
        ECSManager::registerComponentToSystem<LOD>(id);
    }

    void update(float deltaTime) override {

        for (EntityId entity : this->getEntities()) {
            auto lodOpt = ECSManager::getComponentOfEntity<LOD>(entity);
            auto posOpt =
                ECSManager::getComponentOfEntity<Positionable>(entity);

            if (lodOpt.has_value() && posOpt.has_value()) {
                LOD &lod = lodOpt.value();
                Positionable &pos = posOpt.value();

                float distance =
                    glm::length(pos.pos - this->camera.getPosition());

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