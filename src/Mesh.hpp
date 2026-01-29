#include <map>

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
};

class Mesh {
    map<glm::vec3, ushort, VecCompare> verticesIndexed;
    vector<glm::vec3> indexedVertices;
    vector<glm::vec2> textureCoords;
    vector<ushort> indices;

    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint textureBuffer;

    bool useTexture = false;

public:
    Mesh() {}

    Mesh(vector<Triangle> triangles) {
        this->buildVertices(triangles);
        this->buildIndices(triangles);

        this->generateBuffersNoTexture();
    }

    Mesh(vector<Triangle> triangles, map<glm::vec3, glm::vec2, VecCompare> textureCoords) {
        this->buildVertices(triangles);
        this->buildIndices(triangles);
        this->buildTextureCoords(textureCoords);

        this->generateBuffersNoTexture();
        glGenBuffers(1, &this->textureBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
        glBufferData(GL_ARRAY_BUFFER, this->textureCoords.size() * sizeof(glm::vec2),
                    &this->textureCoords[0], GL_STATIC_DRAW);
    }

private:
    void buildVertices(vector<Triangle> triangles) {
        for (Triangle triangle : triangles) {
            if (this->verticesIndexed.find(triangle.a) == this->verticesIndexed.end()) {
                this->verticesIndexed[triangle.a] = this->verticesIndexed.size();
            }
            if (this->verticesIndexed.find(triangle.b) == this->verticesIndexed.end()) {
                this->verticesIndexed[triangle.b] = this->verticesIndexed.size();
            }
            if (this->verticesIndexed.find(triangle.c) == this->verticesIndexed.end()) {
                this->verticesIndexed[triangle.c] = this->verticesIndexed.size();
            }
        }

        for (const auto &pair : this->verticesIndexed) {
            this->indexedVertices.push_back(pair.first);
        }
    }

    void buildIndices(vector<Triangle> triangles) {
        size_t size = triangles.size();
        this->indices.clear();
        this->indices.resize(size * 3);

        for (size_t i = 0; i < size; i++) {
            this->indices[i * 3 + 0] = this->verticesIndexed[triangles[i].a];
            this->indices[i * 3 + 1] = this->verticesIndexed[triangles[i].b];
            this->indices[i * 3 + 2] = this->verticesIndexed[triangles[i].c];
        }
    }

    void buildTextureCoords(map<glm::vec3, glm::vec2, VecCompare> textureCoords) {
        this->useTexture = true;

        for (glm::vec3 vertex : this->indexedVertices) {
            this->textureCoords.push_back(textureCoords[vertex]);
        }
    }

    void generateBuffersNoTexture() {
        glGenBuffers(1, &this->vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, this->indexedVertices.size() * sizeof(glm::vec3),
                    &this->indexedVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &this->elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(ushort),
                    &this->indices[0], GL_STATIC_DRAW);
    }

public:
    void draw() {        
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
        glDisableVertexAttribArray(1);
    }

    void deleteBuffers() {
        glDeleteBuffers(1, &this->vertexBuffer);
        glDeleteBuffers(1, &this->elementBuffer);

        if (this->useTexture) {
            glDeleteBuffers(1, &this->textureBuffer);
        }
    }
};