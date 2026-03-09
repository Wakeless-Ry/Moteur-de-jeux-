#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <glm/ext.hpp>

#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/Scene.h"
#include "src/SceneObject.h"
#include "src/Texture.h"
#include "src/scene_examples.cpp"
#include <src/Camera.h>
#include <src/FileLoader.cpp>

NodeId sphereId;

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

        // controls.addKeyDownCallback(
        //     GLFW_KEY_UP, new KeyCallback([this](float deltaTime) {
        //         this->getCamera().increaseRotationSpeed(deltaTime);
        //     }));

        // controls.addKeyDownCallback(
        //     GLFW_KEY_DOWN, new KeyCallback([this](float deltaTime) {
        //         this->getCamera().decreaseRotationSpeed(deltaTime);
        //     }));
        // controls.addKeyDownCallback(GLFW_KEY_SPACE,
        //                             new KeyCallback([this](float deltaTime) {
        //                                 this->getCamera().up(deltaTime);
        //                             }));
        // controls.addKeyDownCallback(GLFW_KEY_LEFT_SHIFT,
        //                             new KeyCallback([this](float deltaTime) {
        //                                 this->getCamera().down(deltaTime);
        //                             }));

        controls.addKeyDownCallback(
            GLFW_KEY_LEFT, new KeyCallback([this](float deltaTime) {
                this->getScene().transform(sphereId,
                                           translate(deltaTime * 2, 0., 0.));
            }));
        controls.addKeyDownCallback(
            GLFW_KEY_RIGHT, new KeyCallback([this](float deltaTime) {
                this->getScene().transform(sphereId,
                                           translate(-(deltaTime * 2), 0., 0.));
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_UP, new KeyCallback([this](float deltaTime) {
                this->getScene().transform(sphereId,
                                           translate(0., 0., deltaTime * 2));
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_DOWN, new KeyCallback([this](float deltaTime) {
                this->getScene().transform(sphereId,
                                           translate(0., 0., -(deltaTime * 2)));
            }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {}

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

    std::optional<Mesh> sphereMeshOpt =
        FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");

    if (sphereMeshOpt.has_value()) {
        SceneObject sphere("shaders/PBR_sphere_vs.glsl",
                           "shaders/PBR_sphere_fs.glsl", sphereMeshOpt.value());
        sphere.setAlbedo({1., 0., 0.});
        sphereId = engine.getScene().addMesh(sphere);
        engine.getScene().setTransform(sphereId, translate(0, 0, 0).scale(0.2));
    } else {
        std::cout << "Mesh pas chargé correctement" << std::endl;
    }

    // // engine.setScene(getPBRbenchmarkScene(10));
    // engine.setScene(getRustedSphereScene());

    engine.run();

    return 0;
}