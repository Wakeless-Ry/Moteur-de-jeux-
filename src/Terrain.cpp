#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

#include <glm/ext.hpp>
#include <glm/gtc/noise.hpp>

#include "src/Mesh.h"
#include "src/SceneObject.h"
#include "src/Texture.h"

class Terrain {
    struct TerrainChunk {
        Mesh *mesh;
        glm::vec2 offset;
        ushort resolution;
    };

    std::vector<TerrainChunk> terrainChunks;
    std::vector<SceneObject> sceneObjects;

    constexpr static const ushort nombreCases = 128;
    constexpr static const ushort nombreVertices = nombreCases + 1;
    constexpr static const float minX = -2;
    constexpr static const float maxX = 2;
    constexpr static const float minY = minX;
    constexpr static const float maxY = maxX;

    constexpr static const float stepX = (maxX - minX) / nombreCases;
    constexpr static const float stepY = (maxY - minY) / nombreCases;

    float getCase(float val) {
        float c = floor((val - minX) / (maxX - minX) * nombreCases);
        return std::clamp(c, 0.f, (float)(nombreCases - 1));
    }

    Mesh *generateTerrain(glm::vec2 offset, ushort nombreCases,
                          size_t nbOctaves) {

        ushort nombreVertices = nombreCases + 1;

        float stepX = (maxX - minX) / nombreCases;
        float stepY = (maxY - minY) / nombreCases;

        std::vector<glm::vec3> vertices(nombreVertices * nombreVertices);
        std::vector<uint> indices;
        std::vector<glm::vec2> uvs(nombreVertices * nombreVertices);

        // auto computeHeight = [&](float wx, float wz, size_t nbOct) -> float {
        //     float r2 = wx * wx + wz * wz;
        //     float r = sqrtf(r2);
        //     float t = 1.0f - glm::smoothstep(10.0f + 1.5f, 10.0f - 1.5f, r);
        //     return t * (r2 / 100 - 1.0f);
        // };

        auto computeHeight = [&](float wx, float wz, size_t nbOct) -> float {
            float height = 0.0f;
            float amplitude = 1.0f;
            float frequency = 0.2f;
            float totalAmplitude = 0.0f;
            for (size_t octave = 0; octave < nbOct; octave++) {
                height +=
                    glm::perlin(glm::vec2(wx, wz) * frequency) * amplitude;
                totalAmplitude += amplitude;
                amplitude *= 0.5f;
                frequency *= 1.8f;
            }
            return std::max(4 * height / totalAmplitude, 0.f);
        };

        for (ushort i = 0; i < nombreVertices; i++) {
            for (ushort j = 0; j < nombreVertices; j++) {

                float wx = i * stepX + minX + offset.x;
                float wz = j * stepY + minY + offset.y;

                glm::vec3 pos(wx, 0, wz);
                glm::vec2 uv((i + 0.5f) / nombreVertices,
                             (j + 0.5f) / nombreVertices);
                // bord -> 2 octaves
                bool isBorder =
                    (i == 0 || i == nombreCases || j == 0 || j == nombreCases);
                size_t octavesForVertex;
                if (isBorder)
                    octavesForVertex = 2;
                else
                    octavesForVertex = nbOctaves;

                pos.y = computeHeight(wx, wz, octavesForVertex);

                vertices[i * nombreVertices + j] = pos;
                uvs[i * nombreVertices + j] = uv;
            }
        }

        for (ushort i = 0; i < nombreCases; i++) {
            for (ushort j = 0; j < nombreCases; j++) {
                uint a = (i + 0) * nombreVertices + (j + 0);
                uint b = (i + 0) * nombreVertices + (j + 1);
                uint c = (i + 1) * nombreVertices + (j + 0);
                uint d = (i + 1) * nombreVertices + (j + 1);
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(c);
                indices.push_back(b);
                indices.push_back(d);
                indices.push_back(c);
            }
        }

        return new Mesh(vertices, indices, uvs);
    }

    int get_ring(glm::ivec2 chunk, glm::ivec2 center) {
        int dx = std::abs(chunk.x - center.x);
        int dz = std::abs(chunk.y - center.y);

        return std::max(dx, dz);
    }

    void compute_lod(int ring, int &resolution, int &octaves) {
        // "puissance de 2"
        resolution = 128 >> ring;

        if (resolution < 32)
            resolution = 32;

        octaves = 8 - 2 * ring;

        if (octaves < 2)
            octaves = 2;
    }

  public:
    Terrain() {
        const Texture water("assets/textures/water.png");
        const Texture sand("assets/textures/sand.png");
        const Texture grass("assets/textures/grass.png");

        int max_rings = 10;
        int grid_size = 2 * max_rings + 1;
        float chunkSize = maxX - minX;

        std::vector<SceneObject> terrains;
        std::vector<std::vector<TerrainChunk>> grid(
            grid_size, std::vector<TerrainChunk>(grid_size));

        glm::ivec2 centerChunk(0, 0);

        for (int x = -max_rings; x <= max_rings; x++) {
            for (int z = -max_rings; z <= max_rings; z++) {
                glm::ivec2 chunkCoord = centerChunk + glm::ivec2(x, z);

                int ring = get_ring(chunkCoord, centerChunk);

                int resolution, octaves;
                compute_lod(ring, resolution, octaves);

                glm::vec2 offset(chunkCoord.x * chunkSize,
                                 chunkCoord.y * chunkSize);

                Mesh *mesh = generateTerrain(offset, resolution, octaves);

                TerrainChunk chunk = {mesh, offset, (ushort)resolution};
                this->terrainChunks.push_back(chunk);
                grid[x + max_rings][z + max_rings] = chunk;
            }
        }

        for (int x = 0; x < grid_size; x++) {
            for (int z = 0; z < grid_size; z++) {
                SceneObject terrain("shaders/terrain_vs.glsl",
                                    "shaders/terrain_fs.glsl",
                                    *grid[x][z].mesh);

                terrain.addTexture(water, "water");
                terrain.addTexture(sand, "sand");
                terrain.addTexture(grass, "grass");

                terrains.push_back(terrain);
            }
        }

        this->sceneObjects = terrains;
    }

    std::vector<SceneObject> getSceneObjects() { return this->sceneObjects; }

    std::optional<std::pair<float, glm::vec3>>
    getProjectedContact(glm::vec3 pos) {
        TerrainChunk *chunk = nullptr;
        for (auto &tc : terrainChunks) {
            float chunkMinX = tc.offset.x + minX;
            float chunkMaxX = tc.offset.x + maxX;
            float chunkMinZ = tc.offset.y + minY;
            float chunkMaxZ = tc.offset.y + maxY;
            if (pos.x >= chunkMinX && pos.x <= chunkMaxX &&
                pos.z >= chunkMinZ && pos.z <= chunkMaxZ) {
                chunk = &tc;
                break;
            }
        }
        if (!chunk)
            return std::nullopt;

        const std::vector<glm::vec3> &vertices = chunk->mesh->getVertices();
        ushort chunkCases = chunk->resolution;
        ushort chunkVerts = chunkCases + 1;

        float cx =
            (pos.x - (minX + chunk->offset.x)) / (maxX - minX) * chunkCases;
        float cz =
            (pos.z - (minY + chunk->offset.y)) / (maxY - minY) * chunkCases;
        cx = std::clamp(cx, 0.f, (float)(chunkCases - 1));
        cz = std::clamp(cz, 0.f, (float)(chunkCases - 1));

        int i = (int)floor(cx);
        int j = (int)floor(cz);
        float u = cx - i;
        float v = cz - j;

        glm::vec3 a = vertices[(i + 0) * chunkVerts + (j + 0)];
        glm::vec3 b = vertices[(i + 0) * chunkVerts + (j + 1)];
        glm::vec3 c = vertices[(i + 1) * chunkVerts + (j + 0)];
        glm::vec3 d = vertices[(i + 1) * chunkVerts + (j + 1)];

        if (u + v <= 1.0f) {
            glm::vec3 nA = b - a;
            glm::vec3 nB = c - a;
            glm::vec3 normal = glm::cross(nA, nB);
            return {{(1 - u - v) * a.y + v * b.y + u * c.y,
                     glm::normalize(normal)}};
        } else {
            glm::vec3 nA = d - b;
            glm::vec3 nB = c - b;
            glm::vec3 normal = glm::cross(nA, nB);
            return {{(1 - u) * b.y + (u + v - 1) * d.y + (1 - v) * c.y,
                     glm::normalize(normal)}};
        }
    }
};
