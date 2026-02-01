#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <glm/ext.hpp>

#include "src/Texture.hpp"
#include "src/Mesh.hpp"
#include "src/Camera.hpp"
#include "src/Controls.hpp"
#include "src/GameEngine.hpp"

template <typename T> T random(T min, T max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(gen);
    } else if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(gen);
    } else {
        static_assert(
            std::is_integral<T>::value || std::is_floating_point<T>::value,
            "randomBetween only supports integral or floating point types");
    }
}

vector<ushort> flatten(vector<vector<ushort>> &values) {
    vector<ushort> result;

    for (vector<ushort> value : values) {
        result.insert(result.end(), value.begin(), value.end());
    }

    return result;
}

void generateTerrain(uint nombreVertices, vector<Triangle> &triangles, map<glm::vec3, glm::vec2, VecCompare> &textureCoords) {
    const ushort nombreCases = nombreVertices - 1;
    const float minX = -1;
    const float maxX = 1;
    const float minY = -1;
    const float maxY = 1;

    const float stepX = (maxX - minX) / nombreCases;
    const float stepY = (maxY - minY) / nombreCases;

    vector<glm::vec3> indexedVertices;

    for (ushort i = 0; i < nombreVertices; i++) {
        for (ushort j = 0; j < nombreVertices; j++) {
            float half = (nombreVertices - 1) / 2.;
            float iWeight = (1 - abs(i - half) / half) * 0.25;
            float jWeight = (1 - abs(j - half) / half) * 0.25;

            glm::vec3 pos = glm::vec3(i * stepX + minX, 0, j * stepY + minY);

            indexedVertices.push_back(pos);
            textureCoords[pos] = glm::vec2((i + 0.5) / nombreVertices, (j + 0.5) / nombreVertices);
        }
    }

    for (ushort i = 0; i < nombreCases; i++) {
        for (ushort j = 0; j < nombreCases; j++) {
            glm::vec3 a = indexedVertices[(i + 0) * nombreVertices + (j + 0)];
            glm::vec3 b = indexedVertices[(i + 0) * nombreVertices + (j + 1)];
            glm::vec3 c = indexedVertices[(i + 1) * nombreVertices + (j + 0)];
            glm::vec3 d = indexedVertices[(i + 1) * nombreVertices + (j + 1)];

            triangles.push_back(Triangle(a, b, c));
            triangles.push_back(Triangle(b, d, c));
        }
    }
}

Mesh buildTerrain(uint nombreVertices) {
    vector<Triangle> triangles;
    map<glm::vec3, glm::vec2, VecCompare> textureCoords;

    generateTerrain(nombreVertices, triangles, textureCoords);

    Mesh terrain("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl", triangles);
    terrain.addTextureCoords(textureCoords);
    terrain.addTexture("assets/noiseTexture.png", "heightMap");
    terrain.addTexture("assets/grass.png", "grassTexture");
    terrain.addTexture("assets/rock.png", "rockTexture");
    terrain.addTexture("assets/snowrocks.png", "snowTexture");

    return terrain;
}

Mesh buildTerrainWithDeltaVertices(int change) {
    static uint nombreVertices = 32;
    nombreVertices += change;
    if (nombreVertices < 2) {
        nombreVertices = 2;
    }

    return buildTerrain(nombreVertices);
}

void initControls(GameEngine &engine, uint terrainId) {
    Controls &controls = engine.getControls();

    controls.addMouseDeltaCallback([&engine](float dx, float dy) {
        engine.getCamera().rotateWithMouse(dx, dy);
    });

    controls.addKeyPressedCallback(GLFW_KEY_ESCAPE, [&engine](float deltaTime) {
        engine.stopRunning();
    });

    controls.addKeyPressedCallback(GLFW_KEY_M, [&engine](float deltaTime) {
            engine.getCamera().changeMode();
    });

    controls.addKeyDownCallback(GLFW_KEY_W, [&engine](float deltaTime) {
        engine.getCamera().forward(deltaTime);
    });

    controls.addKeyDownCallback(GLFW_KEY_A, [&engine](float deltaTime) {
        engine.getCamera().left(deltaTime);
    });

    controls.addKeyDownCallback(GLFW_KEY_S, [&engine](float deltaTime) {
        engine.getCamera().backward(deltaTime);
    });

    controls.addKeyDownCallback(GLFW_KEY_D, [&engine](float deltaTime) {
        engine.getCamera().right(deltaTime);
    });
    
    controls.addKeyDownCallback(GLFW_KEY_KP_ADD, [&engine, terrainId](float deltaTime) {
        Mesh * terrain = engine.getMesh(terrainId);
        terrain->cleanUp();
        terrain = new Mesh(buildTerrainWithDeltaVertices(1));
        engine.replaceMesh(terrainId, terrain);
    });

    controls.addKeyDownCallback(GLFW_KEY_KP_SUBTRACT, [&engine, terrainId](float deltaTime) {
        Mesh * terrain = engine.getMesh(terrainId);
        terrain->cleanUp();
        terrain = new Mesh(buildTerrainWithDeltaVertices(-1));
        engine.replaceMesh(terrainId, terrain);
    });
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

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    window = glfwCreateWindow(mode->width, mode->height, "Game Engine", monitor, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.");
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    GameEngine engine = GameEngine::newGameEngine(window);
    Camera &camera = engine.getCamera();

    glfwPollEvents();
    glfwSetCursorPos(window, camera.getScreenWidth() / 2, camera.getScreenHeight() / 2);

    Mesh terrain = buildTerrainWithDeltaVertices(0);

    uint terrainId = engine.addMesh(&terrain);

    initControls(engine, terrainId);

    engine.run();

    return 0;
}