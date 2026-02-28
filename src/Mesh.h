#ifndef MESH
#define MESH

#include <vector>

#include "glm/detail/type_vec.hpp"
#include <GL/glew.h>

#include "Camera.h"
#include "Observer.h"
#include "Texture.h"
#include "Transform.h"

using ushort = unsigned short;

class Mesh : public Subject<glm::vec3> {
    GLuint programId;

    std::vector<glm::vec3> indexedVertices;
    std::vector<glm::vec2> textureCoords;
    std::vector<ushort> indices;

    std::vector<glm::vec3> normals;

    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint textureBuffer;
    GLuint normalBuffer;

    glm::vec3 albedo = {0, 0, 0};
    float metallic = 0.f;
    float roughness = 0.f;
    float ao = 1.f;

    bool useTexture = false;
    std::vector<Texture> textures;

    Transform transform;

  public:
    Mesh(const char *vertexShaderPath, const char *fragmentShaderPath,
         const std::vector<glm::vec3> &vertices,
         const std::vector<ushort> &indices);

    GLuint getId();

    const glm::vec3 &getAlbedo() const;
    float getMetallic() const;
    float getRoughness() const;
    float getAo() const;

    void setAlbedo(const glm::vec3 &value);
    void setMetallic(float value);
    void setRoughness(float value);
    void setAo(float value);

    void addTextureCoords(const std::vector<glm::vec2> &textureCoords);
    void addTexture(const char *texturePath, const char *varName);

    void draw(const Camera &camera) const;
    void cleanUp();

    Transform getTransform() const;
    void setTransform(const Transform &transform);
    void setNormals(const std::vector<glm::vec3> &normals);
};

#endif // MESH