#include "SceneObject.h"
#include "lib/shader.hpp"

SceneObject::SceneObject(const char *vertexShaderPath,
                         const char *fragmentShaderPath, const Mesh &mesh)
    : mesh(mesh) {
    this->programId = LoadShaders(vertexShaderPath, fragmentShaderPath);
}

GLuint SceneObject::getId() const { return this->programId; }

const glm::vec3 &SceneObject::getAlbedo() const { return albedo; }
float SceneObject::getMetallic() const { return metallic; }
float SceneObject::getRoughness() const { return roughness; }
float SceneObject::getAo() const { return ao; }

void SceneObject::setAlbedo(const glm::vec3 &value) { albedo = value; }
void SceneObject::setMetallic(float value) { metallic = value; }
void SceneObject::setRoughness(float value) { roughness = value; }
void SceneObject::setAo(float value) { ao = value; }

bool SceneObject::addTexture(const char *texturePath, const char *varName) {
    if (this->mesh.hasUVs()) {
        this->useTexture = true;
        Texture newTexture(texturePath, this->textures.size());
        newTexture.bind(this->programId, varName);
        this->textures.push_back(newTexture);

        return true;
    }

    return false;
}

void SceneObject::draw(const Camera &camera,
                       const std::vector<Light> &lights) const {
    glUseProgram(this->programId);

    glm::mat4 model = this->transform.getMatrix();
    glUniformMatrix4fv(glGetUniformLocation(this->programId, "model"), 1,
                       GL_FALSE, glm::value_ptr(model));

    glm::mat4 view = camera.getView();
    glUniformMatrix4fv(glGetUniformLocation(this->programId, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = camera.getProjection();
    glUniformMatrix4fv(glGetUniformLocation(this->programId, "projection"), 1,
                       GL_FALSE, glm::value_ptr(projection));

    glm::vec3 cameraPos = camera.getPosition();
    glUniform3fv(glGetUniformLocation(this->programId, "camPos"), 1,
                 glm::value_ptr(cameraPos));

    glUniform3fv(glGetUniformLocation(this->programId, "albedo"), 1,
                 glm::value_ptr(this->albedo));
    glUniform1f(glGetUniformLocation(this->programId, "metallic"),
                this->metallic);
    glUniform1f(glGetUniformLocation(this->programId, "roughness"),
                this->roughness);
    glUniform1f(glGetUniformLocation(this->programId, "ao"), this->ao);

    const int MAX_LIGHTS = 10;
    int count = std::min((int)lights.size(), MAX_LIGHTS);

    glUniform1i(glGetUniformLocation(programId, "lightCount"), count);

    for (int i = 0; i < count; ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";
        glUniform3fv(
            glGetUniformLocation(programId, (base + ".position").c_str()), 1,
            glm::value_ptr(lights[i].getLightPos()));

        glUniform3fv(glGetUniformLocation(programId, (base + ".color").c_str()),
                     1, glm::value_ptr(lights[i].getLightColor()));
    }

    this->mesh.draw();
}

void SceneObject::cleanUp() {
    this->mesh.cleanUp();

    if (this->useTexture) {
        for (auto &texture : this->textures) {
            texture.cleanUp();
        }
    }

    glDeleteProgram(this->programId);
    this->programId = 0;
}

Transform SceneObject::getTransform() const { return this->transform; }

void SceneObject::setTransform(const Transform &transform) {
    this->transform = transform;
}