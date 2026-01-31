#ifndef MESH
#define MESH

#include <map>

#include "lib/shader.hpp"

#include "Camera.hpp"

using namespace std;

using ushort = unsigned short;

struct VecCompare {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }

    bool operator()(const glm::vec2& a, const glm::vec2& b) const {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }
};

struct Triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    Triangle() {}
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c): a(a), b(b), c(c) {}
};

class Mesh {
    GLuint programId;

    map<glm::vec3, ushort, VecCompare> verticesIndexed;
    vector<glm::vec3> indexedVertices;
    vector<glm::vec2> textureCoords;
    vector<ushort> indices;

    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint textureBuffer;

    bool useTexture = false;
    vector<Texture> textures;

public:
    Mesh() {}

    Mesh(const char *vertexShaderPath, const char *fragmentShaderPath, const vector<Triangle> &triangles) {
        this->programId = LoadShaders(vertexShaderPath, fragmentShaderPath);
        glUseProgram(this->programId);
        
        this->buildVertices(triangles);
        this->buildIndices(triangles);

        glGenBuffers(1, &this->vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, this->indexedVertices.size() * sizeof(glm::vec3),
                    &this->indexedVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &this->elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(ushort),
                    &this->indices[0], GL_STATIC_DRAW);
    }

private:
    void buildVertices(const vector<Triangle> &triangles) {
        for (Triangle triangle : triangles) {
            this->verticesIndexed[triangle.a];
            this->verticesIndexed[triangle.b];
            this->verticesIndexed[triangle.c];
        }
        
        this->indexedVertices.reserve(this->verticesIndexed.size());
        ushort index = 0;
        for (auto &pair : this->verticesIndexed) {
            pair.second = index++;            
            this->indexedVertices.push_back(pair.first);
        }
    }

    void buildIndices(const vector<Triangle> &triangles) {
        size_t size = triangles.size();
        this->indices.clear();
        this->indices.resize(size * 3);

        for (size_t i = 0; i < size; i++) {
            this->indices[i * 3 + 0] = this->verticesIndexed[triangles[i].a];
            this->indices[i * 3 + 1] = this->verticesIndexed[triangles[i].b];
            this->indices[i * 3 + 2] = this->verticesIndexed[triangles[i].c];
        }
    }

public:
    GLuint getId() {
        return this->programId;
    }

    void addTextureCoords(const map<glm::vec3, glm::vec2, VecCompare> &textureCoords) {
        this->textureCoords.clear();

        for (glm::vec3 vertex : this->indexedVertices) {
            this->textureCoords.push_back(textureCoords.at(vertex));
        }

        glUseProgram(this->programId);
        glGenBuffers(1, &this->textureBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
        glBufferData(GL_ARRAY_BUFFER, this->textureCoords.size() * sizeof(glm::vec2),
                    &this->textureCoords[0], GL_STATIC_DRAW);
    }

    void addTexture(const char * texturePath, const char * varName) {
        this->useTexture = true;
        Texture newTexture(texturePath, this->textures.size());
        newTexture.bind(this->programId, varName);

        this->textures.push_back(newTexture);
    }

    void draw(Camera camera) const {
        glUseProgram(this->programId);

        glm::mat4 model = glm::mat4();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "model"), 1,
                           GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = camera.getView();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));

        glm::mat4 projection = camera.getProjection();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "projection"), 1,
                           GL_FALSE, glm::value_ptr(projection));

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        if (this->useTexture) {
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);

        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_SHORT, (void *)0);

        glDisableVertexAttribArray(0);
        if (this->useTexture) {
            glDisableVertexAttribArray(1);
        }
    }

    void cleanUp() const {
        glDeleteBuffers(1, &this->vertexBuffer);
        glDeleteBuffers(1, &this->elementBuffer);

        if (this->useTexture) {
            glDeleteBuffers(1, &this->textureBuffer);
        }

        glDeleteProgram(this->programId);
    }
};

#endif //MESH