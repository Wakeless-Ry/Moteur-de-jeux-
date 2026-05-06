#ifndef CONTINUOUSLOD
#define CONTINUOUSLOD

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/LOD.h"
#include "src/ecs/components/Positionable.h"
#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>
using uint = unsigned int;

class ContinuousLOD : public System {
  public:
    void update(float deltaTime, const glm::vec3 &cameraPos, float distance) {
        ECSManager &ecs = ECSManager::getManager();

        for (EntityId entity : this->getEntities()) {
            auto lodOpt = ecs.getComponentOfEntity<LOD>(entity);
            auto posOpt = ecs.getComponentOfEntity<Positionable>(entity);

            if (lodOpt.has_value() && posOpt.has_value()) {
                LOD &lod = lodOpt.value();
                Positionable &pos = posOpt.value();

                // float distance = glm::distance(cameraPos, pos.pos);
                // std::cout << "Valeur de la distance = " << distance
                //           << std::endl;

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

        // Recupérer les vertex, indices, normales? , coordonnées UV

        // Créer

        // faire une map pour connaitre les triangles
    }

    void updateNormals(std::vector<glm::vec3> &vertices,
                       const std::vector<uint> &indices,
                       std::vector<glm::vec3> &normals) {
        normals.assign(vertices.size(), glm::vec3(0.0f));

        std::unordered_map<uint, std::vector<size_t>>
            trianglesMap; // liste des indices d'une surface
        std::vector<glm::vec3> trianglesNormals(indices.size() / 3);

        for (size_t i = 0; i < indices.size(); i += 3) {
            uint i0 = indices[i];
            uint i1 = indices[i + 1];
            uint i2 = indices[i + 2];

            trianglesMap[i0].push_back(i / 3);
            trianglesMap[i1].push_back(i / 3);
            trianglesMap[i2].push_back(i / 3);

            glm::vec3 v0 = vertices[i0];
            glm::vec3 v1 = vertices[i1];
            glm::vec3 v2 = vertices[i2];
            glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            trianglesNormals[i / 3] = glm::normalize(faceNormal);
        }

        for (size_t i = 0; i < vertices.size(); i++) {
            std::vector<size_t> triangles = trianglesMap[i];
            std::cout << triangles.size() << std::endl;
            glm::vec3 normal;
            for (size_t otherIndex : triangles) {
                glm::vec3 other = trianglesNormals[otherIndex];
                normal += other;
            }
            normal /= triangles.size();
            normals[i] = glm::normalize(normal);
        }
    }
};

#endif