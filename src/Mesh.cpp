#include "Mesh.h"

#include "lib/shader.hpp"
// VecCompare

bool VecCompare::operator()(const glm::vec3 &a, const glm::vec3 &b) const {
    if (a.x != b.x)
        return a.x < b.x;
    if (a.y != b.y)
        return a.y < b.y;
    return a.z < b.z;
}

bool VecCompare::operator()(const glm::vec2 &a, const glm::vec2 &b) const {
    if (a.x != b.x)
        return a.x < b.x;
    return a.y < b.y;
}

// Mesh

Mesh::Mesh(const char *vertexShaderPath, const char *fragmentShaderPath,
           const std::vector<glm::vec3> &vertices,
           const std::vector<ushort> &indices) {
    this->programId = LoadShaders(vertexShaderPath, fragmentShaderPath);
    glUseProgram(this->programId);

    this->indexedVertices = vertices;
    this->indices = indices;

    glGenBuffers(1, &this->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 this->indexedVertices.size() * sizeof(glm::vec3),
                 &this->indexedVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &this->elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(ushort),
                 &this->indices[0], GL_STATIC_DRAW);
}

GLuint Mesh::getId() { return this->programId; }

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

    glUseProgram(this->programId);
    glGenBuffers(1, &this->textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 this->textureCoords.size() * sizeof(glm::vec2),
                 &this->textureCoords[0], GL_STATIC_DRAW);
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

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    if (this->useTexture && !this->textures.empty()) {
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);

    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_SHORT,
                   (void *)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    if (this->useTexture) {
        glDisableVertexAttribArray(2);
    }
}

void Mesh::cleanUp() {
    glDeleteBuffers(1, &this->vertexBuffer);
    glDeleteBuffers(1, &this->elementBuffer);
    glDeleteBuffers(1, &this->normalBuffer);

    if (this->useTexture) {
        glDeleteBuffers(1, &this->textureBuffer);
        for (auto &texture : this->textures) {
            texture.cleanUp();
        }
    }

    glDeleteProgram(this->programId);

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

    glUseProgram(this->programId);
    glGenBuffers(1, &this->normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(glm::vec3),
                 &this->normals[0], GL_STATIC_DRAW);
}