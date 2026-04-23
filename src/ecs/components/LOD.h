#ifndef LOD_
#define LOD_

#include <glm/ext.hpp>
#include <vector>
#include "src/Mesh.h"

using uint = unsigned int;

struct LOD {
    std::vector<glm::vec3> original_vertices;
    std::vector<uint> original_indices;
    std::vector<glm::vec3> original_normals;
    std::vector<glm::vec2> original_uvs;

    std::vector<glm::vec3> current_vertices;
    std::vector<uint> current_indices;
    std::vector<glm::vec3> current_normals;
    std::vector<glm::vec2> current_uvs;

    float target_lod_level = 1.0f;
    float current_lod_level = 1.0f;

    LOD() {}
    
    LOD(const Mesh& mesh) {
        original_vertices = mesh.getVertices();
        original_indices = mesh.getIndices();
        original_normals = mesh.getNormals();
        original_uvs = mesh.getUvs();
        
        current_vertices = original_vertices;
        current_indices = original_indices;
        current_normals = original_normals;
        current_uvs = original_uvs;
    }
};

#endif