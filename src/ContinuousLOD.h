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
  private:
    static constexpr int maxCollapsePerFrame = 50;
    static constexpr float collapseCostThreshold = 5.0f;

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

                float targetLOD;
                if (distance < 25.0f) {
                    targetLOD = 1.0f;
                } else if (distance < 50.0f) {
                    targetLOD = 0.67f;
                } else if (distance < 75.0f) {
                    targetLOD = 0.33f;
                } else {
                    targetLOD = 0.15f;
                }

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

        if (currentTriCount < target_triangle_count) {
            lod.current_vertices = lod.original_vertices;
            lod.current_indices = lod.original_indices;
            lod.current_normals = lod.original_normals;
            lod.current_uvs = lod.original_uvs;
            currentTriCount = lod.current_indices.size() / 3;
        }

        if (currentTriCount > target_triangle_count) {
            int edgesCollapsed = 0;

            std::vector<std::pair<uint, uint>> edges;
            for (size_t i = 0; i + 2 < lod.current_indices.size(); i += 3) {
                uint i0 = lod.current_indices[i];
                uint i1 = lod.current_indices[i + 1];
                uint i2 = lod.current_indices[i + 2];
                edges.push_back({i0, i1});
                edges.push_back({i1, i2});
                edges.push_back({i0, i2});
            }

            std::sort(edges.begin(), edges.end(), [&](auto &a, auto &b) {
                if (a.first >= lod.current_vertices.size() ||
                    a.second >= lod.current_vertices.size())
                    return false;
                if (b.first >= lod.current_vertices.size() ||
                    b.second >= lod.current_vertices.size())
                    return true;
                return glm::distance(lod.current_vertices[a.first],
                                     lod.current_vertices[a.second]) <
                       glm::distance(lod.current_vertices[b.first],
                                     lod.current_vertices[b.second]);
            });

            for (auto &[v0, v1] : edges) {
                if (lod.current_indices.size() / 3 <= target_triangle_count)
                    break;
                if (edgesCollapsed >= maxCollapsePerFrame)
                    break;
                if (v0 >= lod.current_vertices.size() ||
                    v1 >= lod.current_vertices.size())
                    continue;

                if (tryCollapseEdge(lod, v0, v1))
                    edgesCollapsed++;
            }

            updateNormals(lod.current_vertices, lod.current_indices,
                          lod.current_normals);
        }

        lod.current_lod_level = targetLODLevel;
    }

    bool tryCollapseEdge(LOD &lod, uint v0, uint v1) {
        if (v0 >= lod.current_vertices.size() ||
            v1 >= lod.current_vertices.size() || v0 == v1) {
            return false;
        }

        float cost =
            glm::distance(lod.current_vertices[v0], lod.current_vertices[v1]);
        if (cost > collapseCostThreshold)
            return false;

        lod.current_vertices[v0] =
            (lod.current_vertices[v0] + lod.current_vertices[v1]) * 0.5f;
        if (!lod.current_uvs.empty() && v0 < lod.current_uvs.size() &&
            v1 < lod.current_uvs.size()) {
            lod.current_uvs[v0] =
                (lod.current_uvs[v0] + lod.current_uvs[v1]) * 0.5f;
        }

        for (uint &idx : lod.current_indices) {
            if (idx == v1)
                idx = v0;
            else if (idx > v1)
                idx--;
        }

        auto i = lod.current_indices.begin();
        while (i != lod.current_indices.end()) {
            if (i + 2 >= lod.current_indices.end())
                break;
            uint i0 = *i, i1 = *(i + 1), i2 = *(i + 2);
            if (i0 == i1 || i1 == i2 || i0 == i2)
                i = lod.current_indices.erase(i, i + 3);
            else
                i += 3;
        }

        lod.current_vertices.erase(lod.current_vertices.begin() + v1);
        if (!lod.current_uvs.empty() && v1 < lod.current_uvs.size())
            lod.current_uvs.erase(lod.current_uvs.begin() + v1);

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