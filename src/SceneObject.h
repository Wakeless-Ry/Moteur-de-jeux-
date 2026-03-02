#ifndef SCENE_OBJECT
#define SCENE_OBJECT

#include <vector>

#include <GL/glew.h>
#include <glm/ext.hpp>

#include "Camera.h"
#include "Mesh.h"
#include "Observer.h"
#include "Texture.h"
#include "Transform.h"

using uint = unsigned int;

class SceneObject : public Subject<glm::vec3> {
    GLuint programId;

    Mesh mesh;

    glm::vec3 albedo = {1, 0, 0};
    float metallic = 0.f;
    float roughness = 0.f;
    float ao = 1.f;
    bool useTexture = false;
    std::vector<Texture> textures;
    Transform transform;

  public:
    SceneObject(const char *vertexShaderPath, const char *fragmentShaderPath,
                const Mesh &mesh);

    GLuint getId() const;

    const glm::vec3 &getAlbedo() const;
    float getMetallic() const;
    float getRoughness() const;
    float getAo() const;

    void setAlbedo(const glm::vec3 &value);
    void setMetallic(float value);
    void setRoughness(float value);
    void setAo(float value);

    bool addTexture(const char *texturePath, const char *varName);

    void draw(const Camera &camera) const;

    void cleanUp();

    Transform getTransform() const;
    void setTransform(const Transform &transform);
};

#endif // SCENE_OBJECT