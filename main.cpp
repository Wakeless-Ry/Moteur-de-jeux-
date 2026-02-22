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
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_J,
                                    new KeyCallback([this](float deltaTime) {
                                    }));
    }

    float angle_rotation_soleil = 0;
    float sunSpeed = 2.; 
    float speed_regulator = 2.;
    float earthspeed = 2;
    float moonSpeed = 2;


    float angle_terre_rotation = 0.0f;
    float angle_terre_revolution = 0.0f;
    float angle_lune_revolution = 0.0f;

    void processInput(float deltaTime) override {}
    void update(float deltaTime) override {
        angle_rotation_soleil += 36 * deltaTime;
        this->getScene().setTransform(solarMovement,Transform().rotationY(angle_rotation_soleil));

        angle_terre_revolution += 36 * deltaTime;
        this->getScene().setTransform(earthOrbit,Transform().rotationY(angle_terre_revolution).translate(4,0,0).scale(0.5));

        angle_terre_rotation += 36* deltaTime;
        this->getScene().setTransform(earth,Transform().rotationX(23).rotationY(angle_terre_rotation));

        angle_lune_revolution += 36 * deltaTime;
        this->getScene().setTransform(moon, Transform().rotationY(angle_lune_revolution).translate(3, 0, 0).scale(0.5));

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


void solar_system(Moteur &engine, const Mesh &mesh){
    solarMovement = engine.getScene().addBasicNode();
    sun = engine.getScene().addMeshAsChild(solarMovement,mesh).value();
    earthOrbit = engine.getScene().addBasicNodeAsChild(solarMovement).value();
    earth = engine.getScene().addMeshAsChild(earthOrbit, mesh).value();
    moon = engine.getScene().addMeshAsChild(earthOrbit, mesh).value();
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

    std::optional<Mesh> mesh = FileLoader::buildMeshFromOFF(
        "shaders/sphere_vs.glsl", "shaders/sphere_fs.glsl",
        "assets/unit_sphere_n.off");
    if (mesh.has_value()) {
        solar_system(engine,mesh.value());
    } else {
        std::cout << "Mesh pas chargé correctement" << std::endl;
    }

    engine.run();

    return 0;
}