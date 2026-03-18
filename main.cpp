#include <algorithm>
#include <cmath>
#include <optional>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include "glm/detail/type_vec.hpp"
#include <glm/ext.hpp>
#include <glm/gtc/noise.hpp>

#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/Scene.h"
#include "src/SceneObject.h"
#include "src/Texture.h"
#include <src/Camera.h>
#include <src/FileLoader.cpp>
#include <src/ecs/ECSManager.h>

NodeId cubeId;
const float SPHERE_RADIUS = 0.2;
glm::vec3 pos(0, SPHERE_RADIUS, 0);

glm::vec3 velocity(0., 0., 0.);
const glm::vec3 gravity(0., -9.81, 0.);
const float friction = 0.95;
const float frictionAir = 1.f;

struct TerrainChunk {
    Mesh *mesh;
    glm::vec2 offset;
    ushort resolution;
};

std::vector<TerrainChunk> terrainChunks;

const ushort nombreCases = 128;
const ushort nombreVertices = nombreCases + 1;
const float minX = -2;
const float maxX = 2;
const float minY = minX;
const float maxY = maxX;

const float stepX = (maxX - minX) / nombreCases;
const float stepY = (maxY - minY) / nombreCases;

float getCase(float val) {
    float c = floor((val - minX) / (maxX - minX) * nombreCases);
    return std::clamp(c, 0.f, (float)(nombreCases - 1));
}

std::optional<std::pair<float, glm::vec3>> getContact(glm::vec3 pos) {
    TerrainChunk *chunk = nullptr;
    for (auto &tc : terrainChunks) {
        float chunkMinX = tc.offset.x + minX;
        float chunkMaxX = tc.offset.x + maxX;
        float chunkMinZ = tc.offset.y + minY;
        float chunkMaxZ = tc.offset.y + maxY;
        if (pos.x >= chunkMinX && pos.x <= chunkMaxX && pos.z >= chunkMinZ &&
            pos.z <= chunkMaxZ) {
            chunk = &tc;
            break;
        }
    }
    if (!chunk)
        return std::nullopt;

    const std::vector<glm::vec3> &vertices = chunk->mesh->getVertices();
    ushort chunkCases = chunk->resolution;
    ushort chunkVerts = chunkCases + 1;

    float cx = (pos.x - (minX + chunk->offset.x)) / (maxX - minX) * chunkCases;
    float cz = (pos.z - (minY + chunk->offset.y)) / (maxY - minY) * chunkCases;
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
        return {
            {(1 - u - v) * a.y + v * b.y + u * c.y, glm::normalize(normal)}};
    } else {
        glm::vec3 nA = d - b;
        glm::vec3 nB = c - b;
        glm::vec3 normal = glm::cross(nA, nB);
        return {{(1 - u) * b.y + (u + v - 1) * d.y + (1 - v) * c.y,
                 glm::normalize(normal)}};
    }
}

Mesh *generateTerrain(glm::vec2 offset, ushort nombreCases, size_t nbOctaves) {

    ushort nombreVertices = nombreCases + 1;

    float stepX = (maxX - minX) / nombreCases;
    float stepY = (maxY - minY) / nombreCases;

    std::vector<glm::vec3> vertices(nombreVertices * nombreVertices);
    std::vector<uint> indices;
    std::vector<glm::vec2> uvs(nombreVertices * nombreVertices);

    auto computeHeight = [&](float wx, float wz, size_t nbOct) -> float {
        float height = 0.0f;
        float amplitude = 1.0f;
        float frequency = 0.2f;
        float totalAmplitude = 0.0f;
        for (size_t octave = 0; octave < nbOct; octave++) {
            height += glm::perlin(glm::vec2(wx, wz) * frequency) * amplitude;
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

std::vector<SceneObject> buildTerrains() {
    const Texture water("assets/textures/water.png");
    const Texture sand("assets/textures/sand.png");
    const Texture grass("assets/textures/grass.png");

    int max_rings = 10;
    int grid_size = 2 * max_rings + 1;
    float chunkSize = maxX - minX;

    std::vector<SceneObject> terrains;
    std::vector<std::vector<TerrainChunk>> grid(
        grid_size, std::vector<TerrainChunk>(grid_size));

    glm::ivec2 centerChunk(floor(pos.x / chunkSize), floor(pos.z / chunkSize));

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
            terrainChunks.push_back(chunk);
            grid[x + max_rings][z + max_rings] = chunk;
        }
    }

    for (int x = 0; x < grid_size; x++) {
        for (int z = 0; z < grid_size; z++) {
            SceneObject terrain("shaders/terrain_vs.glsl",
                                "shaders/terrain_fs.glsl", *grid[x][z].mesh);

            terrain.addTexture(water, "water");
            terrain.addTexture(sand, "sand");
            terrain.addTexture(grass, "grass");

            terrains.push_back(terrain);
        }
    }

    return terrains;
}

class Moteur : public GameEngine {
    void init() override {
        glfwPollEvents();
        glfwSetCursorPos(this->getWindow(),
                         this->getCamera().getScreenWidth() / 2.,
                         this->getCamera().getScreenHeight() / 2.);

        Controls &controls = this->getControls();

        controls.addMouseDeltaCallback(
            new MouseMoveCallback([this](float dx, float dy) {
                this->getCamera().rotateWithMouse(dx, dy);
            }));

        controls.addKeyPressedCallback(
            GLFW_KEY_ESCAPE,
            new KeyCallback([this](float deltaTime) { this->stopRunning(); }));

        controls.addKeyPressedCallback(
            GLFW_KEY_M, new KeyCallback([this](float deltaTime) {
                CameraMode mode = this->getCamera().changeMode();
            }));

        controls.addKeyDownCallback(GLFW_KEY_W,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().forward(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_A,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().left(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_S,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().backward(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_D,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().right(deltaTime);
                                    }));

        auto getHorizontalForward = [this]() -> glm::vec3 {
            float yawRadian = glm::radians(this->getCamera().getEulerAngle().y);
            return glm::normalize(
                glm::vec3(sin(yawRadian), 0.f, cos(yawRadian)));
        };

        auto getHorizontalRight = [this, getHorizontalForward]() -> glm::vec3 {
            return glm::normalize(
                glm::cross(getHorizontalForward(), glm::vec3(0.f, 1.f, 0.f)));
        };

        const float speed = 0.2f;

        controls.addKeyDownCallback(
            GLFW_KEY_UP, new KeyCallback([this, getHorizontalForward,
                                          speed](float deltaTime) {
                velocity += getHorizontalForward() * speed;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_SPACE,
            new KeyCallback([this](float deltaTime) { velocity.y += 0.5f; }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        velocity += gravity * deltaTime;
        pos += velocity * deltaTime;
        auto pair = getContact(pos);
        if (pair.has_value()) {
            auto &[hauteur, normal] = pair.value();
            pos.y = std::max(pos.y, SPHERE_RADIUS + hauteur);
            if (pos.y <= SPHERE_RADIUS + hauteur) {
                float tmp = -velocity.y;
                velocity.y = 0;
                velocity += normal * tmp;
                velocity *= pow(1.0 - friction, deltaTime);
            }
        }
        velocity *= exp(-frictionAir * deltaTime);
        this->getScene().setTransform(cubeId, translate(pos).scale(0.2));
        this->getCamera().setTarget(pos);
    }

    void render(float deltaTime) override {}
    void cleanUp() override {}

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {}
};

int initializeGlew() {
    glewExperimental = true;

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    return 0;
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    window =
        glfwCreateWindow(mode->width, mode->height, "Game Engine", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.");
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    int glewInitied = initializeGlew();
    if (glewInitied != 0) {
        return glewInitied;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    Moteur engine(window, width, height);

    std::optional<Mesh> cubeMeshOpt =
        FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");

    if (cubeMeshOpt.has_value()) {
        SceneObject cube("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                         cubeMeshOpt.value());
        Texture rustedAlbedoMap("assets/textures/rustediron2_albedo.png");
        Texture rustedNormalMap("assets/textures/rustediron2_normal.png");
        Texture rustedMetallicMap("assets/textures/rustediron2_metallic.png");
        Texture rustedRoughnessMap("assets/textures/rustediron2_roughness.png");

        cube.addAlbedoMap(rustedAlbedoMap);
        cube.addNormalMap(rustedNormalMap);
        cube.addMetallicMap(rustedMetallicMap);
        cube.addRoughnessMap(rustedRoughnessMap);
        cubeId = engine.getScene().addMesh(cube);
        engine.getScene().setTransform(cubeId, translate(pos).scale(0.2));

        engine.getScene().addLightToScene(
            Light(glm::vec3(0, 20, 0), glm::vec3(10000)));
    } else {
        std::cout << "Mesh pas chargé correctement" << std::endl;
    }

    for (auto &terrain : buildTerrains()) {
        engine.getScene().addMesh(terrain);
    }

    engine.run();

    return 0;
}