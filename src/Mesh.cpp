#include "Mesh.h"
#include "lib/shader.hpp"

Mesh::Mesh(const char *vertexShaderPath, const char *fragmentShaderPath,
           const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices) {
    this->programId = LoadShaders(vertexShaderPath, fragmentShaderPath);
    this->indexedVertices = vertices;
    this->indices = indices;

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 this->indexedVertices.size() * sizeof(glm::vec3),
                 &this->indexedVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // Moved here
    glEnableVertexAttribArray(0);                                  // Moved here

    glGenBuffers(1, &this->elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(uint),
                 &this->indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}

GLuint Mesh::getId() const { return this->programId; }

const glm::vec3 &Mesh::getAlbedo() const { return albedo; }
float Mesh::getMetallic() const { return metallic; }
float Mesh::getRoughness() const { return roughness; }
float Mesh::getAo() const { return ao; }

void Mesh::setAlbedo(const glm::vec3 &value) { albedo = value; }
void Mesh::setMetallic(float value) { metallic = value; }
void Mesh::setRoughness(float value) { roughness = value; }
void Mesh::setAo(float value) { ao = value; }

void Mesh::addTextureCoords(const std::vector<glm::vec2> &textureCoords) {
    this->textureCoords = textureCoords;

    glBindVertexArray(this->vao);
    glGenBuffers(1, &this->textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 this->textureCoords.size() * sizeof(glm::vec2),
                 &this->textureCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Mesh::addTexture(const char *texturePath, const char *varName) {
    this->useTexture = true;
    Texture newTexture(texturePath, this->textures.size());
    newTexture.bind(this->programId, varName);
    this->textures.push_back(newTexture);
}

void Mesh::draw(const Camera &camera) const {
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

    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT,
                   (void *)0);
    glBindVertexArray(0);
}

void Mesh::cleanUp() {
    glDeleteBuffers(1, &this->vertexBuffer);
    glDeleteBuffers(1, &this->elementBuffer);
    glDeleteBuffers(1, &this->normalBuffer);
    glDeleteVertexArrays(1, &this->vao);

    if (this->useTexture) {
        glDeleteBuffers(1, &this->textureBuffer);
        for (auto &texture : this->textures) {
            texture.cleanUp();
        }
    }

    glDeleteProgram(this->programId);

    this->vao = 0;
    this->vertexBuffer = 0;
    this->elementBuffer = 0;
    this->textureBuffer = 0;
    this->programId = 0;
}

Transform Mesh::getTransform() const { return this->transform; }

void Mesh::setTransform(const Transform &transform) {
    this->transform = transform;
}

void Mesh::setNormals(const std::vector<glm::vec3> &normals) {
    this->normals = normals;

    glBindVertexArray(this->vao);
    glGenBuffers(1, &this->normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(glm::vec3),
                 &this->normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}