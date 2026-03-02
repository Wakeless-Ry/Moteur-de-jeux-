#include "Mesh.h"

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices)
    : Mesh(vertices, indices, std::vector<glm::vec3>{},
           std::vector<glm::vec2>{}) {}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices,
           const std::vector<glm::vec3> &normals)
    : Mesh(vertices, indices, normals, std::vector<glm::vec2>{}) {}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices, const std::vector<glm::vec2> &uvs)
    : Mesh(vertices, indices, std::vector<glm::vec3>{}, uvs) {}

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
           const std::vector<uint> &indices,
           const std::vector<glm::vec3> &normals,
           const std::vector<glm::vec2> &uvs)
    : vao(0), vbo(0), nbo(0), ubo(0), ebo(0), vertices(vertices),
      normals(normals), uvs(uvs), indices(indices) {

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3),
                 this->vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (this->hasNormals()) {
        glGenBuffers(1, &this->nbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->nbo);
        glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(glm::vec3),
                     this->normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (this->hasUVs()) {
        glGenBuffers(1, &this->ubo);
        glBindBuffer(GL_ARRAY_BUFFER, this->ubo);
        glBufferData(GL_ARRAY_BUFFER, this->uvs.size() * sizeof(glm::vec2),
                     this->uvs.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(uint),
                 this->indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

bool Mesh::hasNormals() const { return !this->normals.empty(); }

bool Mesh::hasUVs() const { return !this->uvs.empty(); }

void Mesh::draw() const {
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::cleanUp() {
    glDeleteBuffers(1, &this->vbo);
    if (this->hasNormals()) {
        glDeleteBuffers(1, &this->nbo);
    }
    if (this->hasUVs()) {
        glDeleteBuffers(1, &this->ubo);
    }
    glDeleteBuffers(1, &this->ebo);
    glDeleteVertexArrays(1, &this->vao);

    this->vao = 0;
    this->vbo = 0;
    this->nbo = 0;
    this->ubo = 0;
    this->vao = 0;
}