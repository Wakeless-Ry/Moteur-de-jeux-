#ifndef MESH
#define MESH

#include <map>
#include <vector>

#include <GL/glew.h>

#include "glm/gtc/type_ptr.hpp"
#include "lib/shader.hpp"

#include "Camera.hpp"
#include "Observer.hpp"
#include "Texture.hpp"
#include "Transform.hpp"

using ushort = unsigned short;

struct VecCompare {
    bool operator()(const glm::vec3 &a, const glm::vec3 &b) const {
        if (a.x != b.x)
            return a.x < b.x;
        if (a.y != b.y)
            return a.y < b.y;
        return a.z < b.z;
    }

    bool operator()(const glm::vec2 &a, const glm::vec2 &b) const {
        if (a.x != b.x)
            return a.x < b.x;
        return a.y < b.y;
    }
};

class Mesh : public Subject<glm::vec3> {
    GLuint programId;

    map<glm::vec3, ushort, VecCompare> verticesIndexed;
    vector<glm::vec3> indexedVertices;
    vector<glm::vec2> textureCoords;
    vector<ushort> indices;

    vector<glm::vec3> normals;

    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint textureBuffer;
    GLuint normalBuffer;

    bool useTexture = false;
    vector<Texture> textures;

    Transform transform;

  public:
    Mesh(const char *vertexShaderPath, const char *fragmentShaderPath,
         const vector<glm::vec3> &vertices, const vector<ushort> &indices) {
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     this->indices.size() * sizeof(ushort), &this->indices[0],
                     GL_STATIC_DRAW);
    }

  public:
    GLuint getId() { return this->programId; }

    void addTextureCoords(const std::vector<glm::vec2> &textureCoords) {
        this->textureCoords = textureCoords;

        glUseProgram(this->programId);
        glGenBuffers(1, &this->textureBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     this->textureCoords.size() * sizeof(glm::vec2),
                     &this->textureCoords[0], GL_STATIC_DRAW);
    }

    void addTexture(const char *texturePath, const char *varName) {
        this->useTexture = true;
        Texture newTexture(texturePath, 0);
        this->textures.clear();
        this->textures.push_back(newTexture);
    }

    void draw(const Camera &camera) const {
        glUseProgram(this->programId);

        glm::mat4 model = this->transform.getMatrix();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "model"), 1,
                           GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = camera.getView();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "view"), 1,
                           GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = camera.getProjection();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "projection"),
                           1, GL_FALSE, glm::value_ptr(projection));

        glm::vec3 cameraPos = camera.getPosition();
        glUniform3fv(glGetUniformLocation(this->programId, "camPos"), 1,
                     glm::value_ptr(cameraPos));

        std::cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z
                  << std::endl;

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
            this->textures[0].bind(this->programId, "planetTexture");
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

    void cleanUp() {
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

    Transform getTransform() const { return this->transform; }

    void setTransform(const Transform &transform) {
        this->transform = transform;
    }

    void setNormals(const std::vector<glm::vec3> &normals) {
        this->normals = normals;

        glUseProgram(this->programId);
        glGenBuffers(1, &this->normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(glm::vec3),
                     &this->normals[0], GL_STATIC_DRAW);
    }
};

#endif // MESH