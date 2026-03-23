#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include "glm/detail/type_vec.hpp"
#include <glm/ext.hpp>
#include <glm/gtc/noise.hpp>

#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/Physics.h"
#include "src/Scene.h"
#include "src/SceneObject.h"
#include "src/Texture.h"
#include "src/ecs/components/RigidBody.h"
#include "src/ecs/utils.h"
#include <src/Camera.h>
#include <src/FileLoader.cpp>
#include <src/ecs/ECSManager.h>

NodeId cubeId;
EntityId sphereId;
const float SPHERE_RADIUS = 0.2;

auto physics = std::make_shared<Physics>();

const float friction = 0.95;
const float frictionAir = 1.f;

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

        controls.addKeyDownCallback(GLFW_KEY_UP,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().forward(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_LEFT,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().left(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_DOWN,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().backward(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_RIGHT,
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

        const float speed = 0.02f;

        glm::vec3 &force = ECSManager::getManager()
                               .getComponentOfEntity<RigidBody>(sphereId)
                               .value()
                               .get()
                               .force;

        controls.addKeyDownCallback(
            GLFW_KEY_W, new KeyCallback([this, getHorizontalForward, speed,
                                         &force](float deltaTime) {
                force += getHorizontalForward() * speed;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_S, new KeyCallback([this, getHorizontalForward, speed,
                                         &force](float deltaTime) {
                force -= getHorizontalForward() * speed;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_D, new KeyCallback([this, getHorizontalRight, speed,
                                         &force](float deltaTime) {
                force += getHorizontalRight() * speed;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_A, new KeyCallback([this, getHorizontalRight, speed,
                                         &force](float deltaTime) {
                force -= getHorizontalRight() * speed;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_SPACE, new KeyCallback([this, &force](float deltaTime) {
                force.y += 15.f * deltaTime;
            }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        glm::vec3 &pos = ECSManager::getManager()
                             .getComponentOfEntity<Positionable>(sphereId)
                             .value()
                             .get()
                             .pos;

        physics.get()->update(deltaTime);

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

    SystemId physicsId = ECSManager::getManager().registerSystem(physics);
    ECSManager::getManager().registerComponentToSystem<Positionable>(physicsId);
    ECSManager::getManager().registerComponentToSystem<RigidBody>(physicsId);

    sphereId = ECSManager::getManager().generateEntityId();
    ECSManager::getManager().setComponentToEntity(Positionable(), sphereId);
    ECSManager::getManager().setComponentToEntity(RigidBody(), sphereId);

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
        engine.getScene().setTransform(
            cubeId, translate(ECSManager::getManager()
                                  .getComponentOfEntity<Positionable>(sphereId)
                                  .value()
                                  .get()
                                  .pos)
                        .scale(0.2));

        engine.getScene().addLightToScene(
            Light(glm::vec3(0, 20, 0), glm::vec3(10000)));
    } else {
        std::cout << "Mesh pas chargé correctement" << std::endl;
    }

    physics->generateTerrain();
    for (auto &terrain : physics->getTerrain().getSceneObjects()) {
        engine.getScene().addMesh(terrain);
    }
    engine.run();

    return 0;
}