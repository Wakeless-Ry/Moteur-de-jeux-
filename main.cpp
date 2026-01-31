// Include standard headers
#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "src/Texture.hpp"
#include "src/Mesh.hpp"
#include "src/Camera.hpp"
#include "src/Controls.hpp"

using namespace std;
using namespace glm;
using ushort = unsigned short;
using uint = unsigned int;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

Camera camera(800, 600);
Controls controls;

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

void generateTerrain(vector<Triangle> &triangles, map<glm::vec3, glm::vec2, VecCompare> &textureCoords) {
    const ushort nombreVertices = 128;
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

Mesh buildTerrain() {
    vector<Triangle> triangles;
    map<glm::vec3, glm::vec2, VecCompare> textureCoords;

    generateTerrain(triangles, textureCoords);

    Mesh mesh("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl", triangles);
    mesh.addTextureCoords(textureCoords);
    mesh.addTexture("assets/noiseTexture.png", "heightMap");
    mesh.addTexture("assets/grass.png", "grassTexture");
    mesh.addTexture("assets/rock.png", "rockTexture");
    mesh.addTexture("assets/snowrocks.png", "snowTexture");

    return mesh;
}

void initControls() {
    controls.addMouseDeltaCallback([](float dx, float dy) {
        camera.rotateWithMouse(dx, dy);
    });

    controls.addKeyPressedCallback(GLFW_KEY_ESCAPE, [](float deltaTime) {
        glfwSetWindowShouldClose(window, true);
    });

    controls.addKeyPressedCallback(GLFW_KEY_M, [](float deltaTime) {
            camera.changeMode();
    });

    controls.addKeyDownCallback(GLFW_KEY_W, [](float deltaTime) {
        camera.forward(deltaTime);
    });

    controls.addKeyDownCallback(GLFW_KEY_A, [](float deltaTime) {
        camera.left(deltaTime);
    });

    controls.addKeyDownCallback(GLFW_KEY_S, [](float deltaTime) {
        camera.backward(deltaTime);
    });

    controls.addKeyDownCallback(GLFW_KEY_D, [](float deltaTime) {
        camera.right(deltaTime);
    });
}

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                   GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(mode->width, mode->height, "TP1 - GLFW", monitor, NULL);
    if (window == NULL) {
        fprintf(
            stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    camera.setScreenWidth(mode->width);
    camera.setScreenHeight(mode->height);

    initControls();

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
    glfwPollEvents();
    glfwSetCursorPos(window, camera.getScreenWidth() / 2, camera.getScreenHeight() / 2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    Mesh terrain = buildTerrain();

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do {
        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        controls.processInput(window, camera, deltaTime);
        camera.update(deltaTime);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrain.draw(camera);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    terrain.cleanUp();
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}