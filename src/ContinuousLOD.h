#ifndef CONTINUOUSLOD
#define CONTINUOUSLOD

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/LOD.h"
#include "src/ecs/components/Positionable.h"
#include <iostream>
#include <optional>
#include <queue>
#include <set>
using uint = unsigned int;

struct Edge {
    uint v0, v1;
    float cost;

    bool operator>(const Edge &other) const { return cost > other.cost; }
};

class ContinuousLOD : public System {
  public:
    float calculateEdgeCost(const glm::vec3 &v0, const glm::vec3 &v1) {
        return glm::distance(v0, v1);
    }

    void collapseEdge(LOD &lod, uint v0, uint v1) {
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
            uint i0 = *i;
            uint i1 = *(++i);
            uint i2 = *(++i);

            if (i0 == i1 || i1 == i2 || i0 == i2) {
                i = lod.current_indices.erase(i - 2);
                i = lod.current_indices.erase(i);
                i = lod.current_indices.erase(i);
            } else {
                ++i;
            }
        }
    }

    void updateNormals(std::vector<glm::vec3> &vertices,
                       const std::vector<uint> &indices,
                       std::vector<glm::vec3> &normals) {

        normals.assign(vertices.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < indices.size(); i += 3) {
            uint i0 = indices[i];
            uint i1 = indices[i + 1];
            uint i2 = indices[i + 2];

            glm::vec3 v0 = vertices[i0];
            glm::vec3 v1 = vertices[i1];
            glm::vec3 v2 = vertices[i2];

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            normals[i0] += normal;
            normals[i1] += normal;
            normals[i2] += normal;
        }

        for (auto &normal : normals) {
            if (glm::length(normal) > 0.0f) {
                normal = glm::normalize(normal);
            }
        }
    }

    void buildCollapseQueue(
        const std::vector<glm::vec3> &vertices,
        const std::vector<uint> &indices,
        std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>>
            &collapseQueue) {

        std::set<std::pair<uint, uint>> uniqueEdges;

        for (size_t i = 0; i < indices.size(); i += 3) {
            uint i0 = indices[i];
            uint i1 = indices[i + 1];
            uint i2 = indices[i + 2];

            auto edge1 = std::make_pair(std::min(i0, i1), std::max(i0, i1));
            auto edge2 = std::make_pair(std::min(i1, i2), std::max(i1, i2));
            auto edge3 = std::make_pair(std::min(i2, i0), std::max(i2, i0));

            uniqueEdges.insert(edge1);
            uniqueEdges.insert(edge2);
            uniqueEdges.insert(edge3);
        }

        for (const auto &edge : uniqueEdges) {
            float cost =
                calculateEdgeCost(vertices[edge.first], vertices[edge.second]);
            collapseQueue.push({edge.first, edge.second, cost});
        }
    }

    void simplifyMesh(LOD &lod, float targetLODLevel) {

        uint originalTriangleCount = lod.original_indices.size() / 3;
        uint targetTriangleCount =
            static_cast<uint>(originalTriangleCount * targetLODLevel);
        uint currentTriangleCount = lod.current_indices.size() / 3;

        std::cout << "=== LOD Simplification ===" << std::endl;
        std::cout << "Original triangles: " << originalTriangleCount
                  << std::endl;
        std::cout << "Target triangles: " << targetTriangleCount
                  << " (LOD level: " << targetLODLevel << ")" << std::endl;
        std::cout << "Current triangles BEFORE: " << currentTriangleCount
                  << std::endl;

        if (currentTriangleCount > targetTriangleCount) {
            std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>>
                collapseQueue;
            buildCollapseQueue(lod.current_vertices, lod.current_indices,
                               collapseQueue);

            uint edgesCollapsed = 0;

            while (currentTriangleCount > targetTriangleCount &&
                   !collapseQueue.empty()) {
                Edge cheapestEdge = collapseQueue.top();
                collapseQueue.pop();

                collapseEdge(lod, cheapestEdge.v0, cheapestEdge.v1);
                updateNormals(lod.current_vertices, lod.current_indices,
                              lod.current_normals);

                currentTriangleCount = lod.current_indices.size() / 3;
                edgesCollapsed++;
            }

            std::cout << "Current triangles AFTER: " << currentTriangleCount
                      << std::endl;
            std::cout << "Edges collapsed: " << edgesCollapsed << std::endl;
            std::cout << "========================" << std::endl;

            lod.current_lod_level = targetLODLevel;
        } else {
            std::cout << "No simplification needed (current <= target)"
                      << std::endl;
            std::cout << "========================" << std::endl;
        }
    }

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

                std::cout << "Distance: " << distance
                          << " | Target LOD: " << targetLOD
                          << " | Current LOD: " << lod.current_lod_level
                          << std::endl;

                lod.target_lod_level = targetLOD;

                if (std::abs(lod.target_lod_level - lod.current_lod_level) >
                    0.05f) {
                    std::cout << "LOD change detected, simplifying..."
                              << std::endl;
                    simplifyMesh(lod, lod.target_lod_level);
                }
            }
        }
    }
};

#endif