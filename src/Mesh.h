#ifndef MESH
#define MESH

#include <vector>

#include <GL/glew.h>
#include <glm/ext.hpp>

class Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint nbo;
    GLuint ubo;
    GLuint ebo;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<uint> indices;

  public:
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices);
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices,
         const std::vector<glm::vec3> &normals);
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices, const std::vector<glm::vec2> &uvs);
    Mesh(const std::vector<glm::vec3> &vertices,
         const std::vector<uint> &indices,
         const std::vector<glm::vec3> &normals,
         const std::vector<glm::vec2> &uvs);

    bool hasNormals() const;
    bool hasUVs() const;

    void draw() const;
    void cleanUp();
};

#endif // MESH