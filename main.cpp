#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <glm/ext.hpp>

#include "src/Controls.hpp"
#include "src/GameEngine.hpp"
#include "src/Mesh.hpp"
#include "src/Scene.hpp"
#include "src/Texture.hpp"
#include "src/Transform.hpp"
#include <src/Camera.hpp>
#include <src/FileLoader.hpp>

NodeId solarMovement;
NodeId sun;
NodeId earthOrbit;
NodeId earth;
NodeId moon;

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

        controls.addKeyDownCallback(GLFW_KEY_UP,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().up_arrow(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_DOWN,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().down_arrow(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_H,
                                    new KeyCallback([this](float deltaTime) {
                                        speedRegulator += 10.f * deltaTime;
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_J,
                                    new KeyCallback([this](float deltaTime) {
                                        speedRegulator -= 10.f * deltaTime;
                                    }));
    }

    float speedRegulator = 30.f;

    float earthYearRatio = 1.0f;
    float moonMonthRatio = 12.37f;

    float earthSelfRotationRatio = 365.f;
    float moonSelfRotationRatio = 12.37f;

    float angle_sun = 0.f;
    float angle_earth_revolution = 0.f;
    float angle_earth_rotation = 0.f;
    float angle_moon_revolution = 0.f;
    float angle_moon_rotation = 0.f;

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        angle_sun += speedRegulator * 0.2f * deltaTime;
        this->getScene().setTransform(
            sun, Transform().rotationY(angle_sun).scale(1.5f));

        angle_earth_revolution += speedRegulator * earthYearRatio * deltaTime;
        this->getScene().setTransform(earthOrbit,
                                      Transform()
                                          .rotationY(angle_earth_revolution)
                                          .translate(4.f, 0.f, 0.f));

        angle_earth_rotation +=
            speedRegulator * earthSelfRotationRatio * deltaTime;

        this->getScene().setTransform(earth,
                                      Transform()
                                          .rotationX(23.f)
                                          .rotationY(angle_earth_rotation)
                                          .scale(0.6f));

        angle_moon_revolution +=
            speedRegulator * earthYearRatio * moonMonthRatio * deltaTime;

        this->getScene().setTransform(moon,
                                      Transform()
                                          .rotationY(angle_moon_revolution)
                                          .translate(1.5f, 0.f, 0.f)
                                          .scale(0.25f));
        angle_moon_rotation = angle_moon_revolution;
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

void solar_system(Moteur &engine, const Mesh &sunMesh, const Mesh &earthMesh,
                  const Mesh &moonMesh) {
    solarMovement = engine.getScene().addBasicNode();
    sun = engine.getScene().addMeshAsChild(solarMovement, sunMesh).value();
    earthOrbit = engine.getScene().addBasicNodeAsChild(solarMovement).value();
    earth = engine.getScene().addMeshAsChild(earthOrbit, earthMesh).value();
    moon = engine.getScene().addMeshAsChild(earthOrbit, moonMesh).value();
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

    std::optional<Mesh> sunMeshOpt = FileLoader::buildMeshFromOBJ(
        "shaders/sphere_vs.glsl", "shaders/sphere_fs.glsl",
        "assets/sphere.obj");
    std::optional<Mesh> earthMeshOpt = FileLoader::buildMeshFromOBJ(
        "shaders/sphere_vs.glsl", "shaders/sphere_fs.glsl",
        "assets/sphere.obj");
    std::optional<Mesh> moonMeshOpt = FileLoader::buildMeshFromOBJ(
        "shaders/sphere_vs.glsl", "shaders/sphere_fs.glsl",
        "assets/sphere.obj");

    if (sunMeshOpt.has_value() && earthMeshOpt.has_value() &&
        moonMeshOpt.has_value()) {
        Mesh sunMesh = sunMeshOpt.value();
        Mesh earthMesh = earthMeshOpt.value();
        Mesh moonMesh = moonMeshOpt.value();

        sunMesh.addTexture("assets/sun.jpg", "planetTexture");
        earthMesh.addTexture("assets/earth.jpg", "planetTexture");
        moonMesh.addTexture("assets/moon.png", "planetTexture");

        solar_system(engine, sunMesh, earthMesh, moonMesh);
    } else {
        std::cout << "Mesh pas chargé correctement" << std::endl;
    }

    engine.run();

    return 0;
}