
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    void setData(const std::vector<glm::vec3> &vertices,
                 const std::vector<glm::vec2> &uvs,
                 const std::vector<unsigned int> &indices);

    void upload();
    void draw() const;
    void clear();

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint uvbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
};

Mesh::Mesh()
{
    glGenVertexArrays(1, &vao);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &uvbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void Mesh::setData(const std::vector<glm::vec3> &vertices,
                   const std::vector<glm::vec2> &uvs,
                   const std::vector<unsigned int> &indices)
{
    if (vbo != 0)
        glDeleteBuffers(1, &vbo);
    if (uvbo != 0)
        glDeleteBuffers(1, &uvbo);
    if (ebo != 0)
        glDeleteBuffers(1, &ebo);
    indexCount = static_cast<GLsizei>(indices.size());

    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(glm::vec3),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &uvbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvbo);
    glBufferData(GL_ARRAY_BUFFER,
                 uvs.size() * sizeof(glm::vec2),
                 uvs.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::draw() const
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}
