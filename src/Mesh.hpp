#ifndef MESH
#define MESH

#include <map>
#include <vector>
#include <fstream>

#include <GL/glew.h>

#include "glm/gtc/type_ptr.hpp"
#include "lib/shader.hpp"

#include "Camera.hpp"
#include "Observer.hpp"
#include "Texture.hpp"
#include "Transform.hpp"

using namespace std;

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

struct Triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    Triangle() {}
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) : a(a), b(b), c(c) {}
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

    bool useTexture = false;
    vector<Texture> textures;

  public:
    Mesh() {}

    Mesh(const char *vertexShaderPath, const char *fragmentShaderPath,
         const vector<Triangle> &triangles) {
        this->programId = LoadShaders(vertexShaderPath, fragmentShaderPath);
        glUseProgram(this->programId);

        this->buildVertices(triangles);
        this->buildIndices(triangles);

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
    GLuint getId() { return this->programId; }

    void addTextureCoords(
        const map<glm::vec3, glm::vec2, VecCompare> &textureCoords) {
        this->textureCoords.clear();

        for (glm::vec3 vertex : this->indexedVertices) {
            this->textureCoords.push_back(textureCoords.at(vertex));
        }

        glUseProgram(this->programId);
        glGenBuffers(1, &this->textureBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     this->textureCoords.size() * sizeof(glm::vec2),
                     &this->textureCoords[0], GL_STATIC_DRAW);
    }

    void addTexture(const char *texturePath, const char *varName) {
        this->useTexture = true;
        Texture newTexture(texturePath, this->textures.size());
        newTexture.bind(this->programId, varName);

        this->textures.push_back(newTexture);
    }

    void draw(const Camera &camera) const {
        glUseProgram(this->programId);

        glm::mat4 model = glm::mat4();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "model"), 1,
                           GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = camera.getView();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "view"), 1,
                           GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = camera.getProjection();
        glUniformMatrix4fv(glGetUniformLocation(this->programId, "projection"),
                           1, GL_FALSE, glm::value_ptr(projection));

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        if (this->useTexture) {
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, this->textureBuffer);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBuffer);

        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_SHORT,
                       (void *)0);

        glDisableVertexAttribArray(0);
        if (this->useTexture) {
            glDisableVertexAttribArray(1);
        }
    }

    void cleanUp() {
        glDeleteBuffers(1, &this->vertexBuffer);
        glDeleteBuffers(1, &this->elementBuffer);

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

    void transformNode(const Transform &transform) {
        // TODO
    }

    void openOFF( std::string const & filename,
                std::vector<glm::vec3> & o_vertices,
                std::vector<glm::vec3> & o_normals,
                std::vector< Triangle > & o_triangles,
                bool load_normals = true )
    {
        std::ifstream myfile;
        myfile.open(filename.c_str());
        if (!myfile.is_open())
        {
            std::cout << filename << " cannot be opened" << std::endl;
            return;
        }

        std::string magic_s;

        myfile >> magic_s;

        if( magic_s != "OFF" )
        {
            std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
            myfile.close();
            exit(1);
        }

        int n_vertices , n_faces , dummy_int;
        myfile >> n_vertices >> n_faces >> dummy_int;

        o_vertices.clear();
        o_normals.clear();

        for( int v = 0 ; v < n_vertices ; ++v )
        {
            float x , y , z ;

            myfile >> x >> y >> z ;
            o_vertices.push_back( glm::vec3( x , y , z ) );

            if( load_normals ) {
                myfile >> x >> y >> z;
                o_normals.push_back( glm::vec3( x , y , z ) );
            }
        }

        o_triangles.clear();
        for( int f = 0 ; f < n_faces ; ++f )
        {
            int n_vertices_on_face;
            myfile >> n_vertices_on_face;

            if( n_vertices_on_face == 3 )
            {
                unsigned int _v1 , _v2 , _v3;
                myfile >> _v1 >> _v2 >> _v3;

            o_triangles.push_back(
                Triangle(o_vertices[_v1],o_vertices[_v2],o_vertices[_v3]
                )
            );
                }
            else if( n_vertices_on_face == 4 )
            {
                unsigned int _v1 , _v2 , _v3 , _v4;
                myfile >> _v1 >> _v2 >> _v3 >> _v4;

                o_triangles.push_back(
                    Triangle(o_vertices[_v1], o_vertices[_v2], o_vertices[_v3])
                );
                o_triangles.push_back(
                    Triangle(o_vertices[_v1], o_vertices[_v3], o_vertices[_v4])
                );

            }
            else
            {
                std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
                myfile.close();
                exit(1);
            }
        }

    }

};

#endif // MESH