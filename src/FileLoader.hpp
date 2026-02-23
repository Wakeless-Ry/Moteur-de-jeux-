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

    static std::optional<Mesh> buildMeshFromOBJ(const char *vertexShaderPath,
                                                const char *fragmentShaderPath,
                                                const char *objFile) {
        printf("Loading OBJ file %s...\n", objFile);

        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<glm::vec3> temp_vertices;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;

        FILE *file = fopen(objFile, "r");
        if (file == NULL) {
            printf("Impossible to open the file!\n");
            return std::nullopt;
        }

        while (true) {
            char lineHeader[128];
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break;

            if (strcmp(lineHeader, "v") == 0) {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                temp_vertices.push_back(vertex);
            } else if (strcmp(lineHeader, "vt") == 0) {
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                uv.y = -uv.y;
                temp_uvs.push_back(uv);
            } else if (strcmp(lineHeader, "vn") == 0) {
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                temp_normals.push_back(normal);
            } else if (strcmp(lineHeader, "f") == 0) {
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                    &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                    &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                    &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                if (matches != 9) {
                    printf("File can't be read by our simple parser :-(\n");
                    fclose(file);
                    return std::nullopt;
                }
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
                uvIndices.push_back(uvIndex[1]);
                uvIndices.push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            } else {
                char buffer[1000];
                fgets(buffer, 1000, file);
            }
        }

        fclose(file);


        // Center geometry at origin
        if (!temp_vertices.empty()) {
            glm::vec3 centroid(0.0f);
            for (const auto &v : temp_vertices) centroid += v;
            centroid /= static_cast<float>(temp_vertices.size());
            for (auto &v : temp_vertices) v -= centroid;
        }

        // Build triangles
        std::vector<Triangle> triangles;
        for (size_t i = 0; i < vertexIndices.size(); i += 3) {
            glm::vec3 v0 = temp_vertices[vertexIndices[i] - 1];
            glm::vec3 v1 = temp_vertices[vertexIndices[i + 1] - 1];
            glm::vec3 v2 = temp_vertices[vertexIndices[i + 2] - 1];
            triangles.push_back(Triangle(v0, v1, v2));
        }

        Mesh mesh(vertexShaderPath, fragmentShaderPath, triangles);

        // Set normals
        std::vector<glm::vec3> mesh_normals;
        for (size_t i = 0; i < normalIndices.size(); ++i) {
            mesh_normals.push_back(temp_normals[normalIndices[i] - 1]);
        }
        mesh.setNormals(mesh_normals);

        // Set texture coordinates
        std::map<glm::vec3, glm::vec2, VecCompare> mesh_texcoords;
        for (size_t i = 0; i < uvIndices.size(); ++i) {
            glm::vec3 vertex = temp_vertices[vertexIndices[i] - 1];
            glm::vec2 uv = temp_uvs[uvIndices[i] - 1];
            mesh_texcoords[vertex] = uv;
        }
        if (!mesh_texcoords.empty()) {
            mesh.addTextureCoords(mesh_texcoords);
        }

        return mesh;
    }


};

#endif // FILE_LOADER