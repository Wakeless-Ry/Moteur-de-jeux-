#ifndef FILE_LOADER
#define FILE_LOADER

#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include <glm/ext.hpp>

#include "Mesh.hpp"

class FileLoader {
  public:
    static optional<Mesh> buildMeshFromOFF(const char *vertexShaderPath,
                                           const char *fragmentShaderPath,
                                           const char *offFile,
                                           bool load_normals = true) {
        std::ifstream myfile;

        myfile.open(offFile);

        if (!myfile.is_open()) {
            std::cout << offFile << " cannot be opened" << std::endl;
            return std::nullopt;
        }

        std::string magic_s;

        myfile >> magic_s;

        if (magic_s != "OFF") {
            std::cout << magic_s << " != OFF : Not a true OFF file"
                      << std::endl;
            myfile.close();
            return std::nullopt;
        }

        int n_vertices, n_faces, dummy_int;
        myfile >> n_vertices >> n_faces >> dummy_int;

        std::vector<glm::vec3> o_vertices;
        std::vector<glm::vec3> o_normals;
        std::vector<Triangle> o_triangles;

        o_vertices.clear();
        o_normals.clear();

        for (int v = 0; v < n_vertices; ++v) {
            float x, y, z;

            myfile >> x >> y >> z;
            o_vertices.push_back(glm::vec3(x, y, z));

            if (load_normals) {
                myfile >> x >> y >> z;
                o_normals.push_back(glm::vec3(x, y, z));
            }
        }

        o_triangles.clear();
        for (int f = 0; f < n_faces; ++f) {
            int n_vertices_on_face;
            myfile >> n_vertices_on_face;

            if (n_vertices_on_face == 3) {
                unsigned int _v1, _v2, _v3;
                myfile >> _v1 >> _v2 >> _v3;

                o_triangles.push_back(Triangle(o_vertices[_v1], o_vertices[_v2],
                                               o_vertices[_v3]));
            } else if (n_vertices_on_face == 4) {
                unsigned int _v1, _v2, _v3, _v4;
                myfile >> _v1 >> _v2 >> _v3 >> _v4;

                o_triangles.push_back(Triangle(o_vertices[_v1], o_vertices[_v2],
                                               o_vertices[_v3]));
                o_triangles.push_back(Triangle(o_vertices[_v1], o_vertices[_v3],
                                               o_vertices[_v4]));

            } else {
                std::cout << "We handle ONLY *.off files with 3 or 4 vertices "
                             "per face"
                          << std::endl;
                myfile.close();
                return std::nullopt;
            }
        }

        Mesh mesh(vertexShaderPath, fragmentShaderPath, o_triangles);
        mesh.setNormals(o_normals);
        return mesh;
    }
};

#endif // FILE_LOADER