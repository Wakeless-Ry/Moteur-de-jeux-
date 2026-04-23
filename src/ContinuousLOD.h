#ifndef CONTINUOUSLOD
#define CONTINUOUSLOD

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/LOD.h"
#include "src/ecs/components/Positionable.h"
#include <iostream>
using uint = unsigned int;

class ContinuousLOD : public System {
  public:
    void update(float deltaTime, const glm::vec3 &cameraPos) {
        ECSManager &ecs = ECSManager::getManager();

        for (EntityId entity : this->getEntities()) {
            auto lodOpt = ecs.getComponentOfEntity<LOD>(entity);
            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);

            if (lodOpt.has_value() && posOpt.has_value()) {
                LOD &lod = lodOpt.value();
                Positionable &pos = posOpt.value();

                float distance = glm::distance(cameraPos, pos.pos);

                float maxDistance = 100.0f;
                float targetLOD =
                    std::max(0.0f, 1.0f - (distance / maxDistance));

                lod.target_lod_level = targetLOD;

                if (std::abs(lod.target_lod_level - lod.current_lod_level) >
                    0.05f) {
                    simplifyMesh(lod, lod.target_lod_level);
                }
            }
        }
    }

  private:
    void simplifyMesh(LOD &lod, float targetLODLevel) {
        uint original_triangle_count = lod.original_indices.size() / 3;
        uint target_triangle_count =
            static_cast<uint>(original_triangle_count * targetLODLevel);
        uint currentTriCount = lod.current_indices.size() / 3;

        if (currentTriCount > target_triangle_count) {
            int edgesCollapsed = 0;

            while (currentTriCount > target_triangle_count &&
                   lod.current_indices.size() >= 6) {
                bool collapsedOne = false;

                for (size_t i = 0;
                     i < lod.current_indices.size() && !collapsedOne; i += 3) {
                    if (i + 2 >= lod.current_indices.size())
                        break;

                    uint i0 = lod.current_indices[i];
                    uint i1 = lod.current_indices[i + 1];
                    uint i2 = lod.current_indices[i + 2];

                    if (tryCollapseEdge(lod, i0, i1)) {
                        collapsedOne = true;
                        edgesCollapsed++;
                    }
                }

                if (!collapsedOne)
                    break;
                currentTriCount = lod.current_indices.size() / 3;
            }

            updateNormals(lod.current_vertices, lod.current_indices,
                          lod.current_normals);
            lod.current_lod_level = targetLODLevel;
        }
    }

    bool tryCollapseEdge(LOD &lod, uint v0, uint v1) {
        if (v0 >= lod.current_vertices.size() ||
            v1 >= lod.current_vertices.size()) {
            return false;
        }
        float cost =
            glm::distance(lod.current_vertices[v0], lod.current_vertices[v1]);

        if (cost > 2.0f)
            return false;

        lod.current_vertices[v0] =
            (lod.current_vertices[v0] + lod.current_vertices[v1]) * 0.5f;

        for (uint &idx : lod.current_indices) {
            if (idx == v1) {
                idx = v0;
            } else if (idx > v1) {
                idx--;
            }
        }

        auto i = lod.current_indices.begin();
        while (i != lod.current_indices.end()) {
            if (i + 2 >= lod.current_indices.end())
                break;

            uint i0 = *i;
            uint i1 = *(i + 1);
            uint i2 = *(i + 2);

            if (i0 == i1 || i1 == i2 || i0 == i2) {
                i = lod.current_indices.erase(i, i + 3);
            } else {
                i += 3;
            }
        }

        return true;
    }

    void updateNormals(std::vector<glm::vec3> &vertices,
                       const std::vector<uint> &indices,
                       std::vector<glm::vec3> &normals) {
        normals.assign(vertices.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < indices.size(); i += 3) {
            uint i0 = indices[i];
            uint i1 = indices[i + 1];
            uint i2 = indices[i + 2];

            if (i0 >= vertices.size() || i1 >= vertices.size() ||
                i2 >= vertices.size())
                continue;

            glm::vec3 v0 = vertices[i0];
            glm::vec3 v1 = vertices[i1];
            glm::vec3 v2 = vertices[i2];
            glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            normals[i0] += faceNormal;
            normals[i1] += faceNormal;
            normals[i2] += faceNormal;
        }

        for (auto &normal : normals) {
            if (glm::length(normal) > 0.0f) {
                normal = glm::normalize(normal);
            }
        }
    }
};

#endif