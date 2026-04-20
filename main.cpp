#include "src/SceneObject.h"
#include "src/ecs/components/VerletBody.h"
#include <memory>
#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow *window;

#include "src/FileLoader.cpp"
#include "src/GameEngine.h"
#include "src/Scene.h"
#include "src/Verlet.h"
#include "src/Write_csv.h"
#include "src/ecs/utils.h"

const float BALL_RADIUS = 0.2;

auto verlet = std::make_shared<Verlet>();
auto write_csv = std::make_shared<Write_CSV>();

class Moteur : public GameEngine {
    NodeId ballSubScene;
    std::optional<SceneObject> ballMesh;
    std::vector<NodeId> ballIds;
    std::vector<EntityId> ballEntityIds;

    void spawnBall(glm::vec3 pos, glm::vec3 direction, float speed,
                   float deltaTime) {
        NodeId ballId =
            this->getScene()
                .addMeshAsChild(this->ballSubScene, this->ballMesh.value())
                .value();
        this->ballIds.push_back(ballId);

        EntityId ballEntityId = ECSManager::getManager().generateEntityId();
        this->ballEntityIds.push_back(ballEntityId);

        ECSManager::getManager().setComponentToEntity(Positionable(pos),
                                                      ballEntityId);
        ECSManager::getManager().setComponentToEntity(
            VerletBody(pos, direction * speed * deltaTime, BALL_RADIUS),
            ballEntityId);
    }

    void spawnOrbitalBall(glm::vec3 pos, glm::vec3 tangentDirection,
                          float deltaTime) {
        NodeId ballId =
            this->getScene()
                .addMeshAsChild(this->ballSubScene, this->ballMesh.value())
                .value();
        this->ballIds.push_back(ballId);

        EntityId ballEntityId = ECSManager::getManager().generateEntityId();
        this->ballEntityIds.push_back(ballEntityId);

        glm::vec3 orbitalVelocity =
            verlet->calculateOrbitalVelocity(pos, tangentDirection);

        glm::vec3 initialAcceleration = orbitalVelocity * deltaTime;
        ECSManager::getManager().setComponentToEntity(Positionable(pos),
                                                      ballEntityId);
        ECSManager::getManager().setComponentToEntity(
            VerletBody(pos - initialAcceleration, initialAcceleration,
                       BALL_RADIUS),
            ballEntityId);
    }

    void init() override {
        glfwPollEvents();
        glfwSetCursorPos(this->getWindow(),
                         this->getCamera().getScreenWidth() / 2.,
                         this->getCamera().getScreenHeight() / 2.);

        Controls &controls = this->getControls();
        this->getCamera().setPosition({10, 10, 10});
        this->getCamera().setTranslationSpeed(5);
        this->getCamera().changeMode();

        glm::vec3 orbitalCenter = {0, 25, 0};
        float orbitalMass = 1000.0f;
        verlet->setOrbitalBody(orbitalCenter, orbitalMass);

        this->ballSubScene = this->getScene().addBasicNode();

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

        controls.addKeyDownCallback(GLFW_KEY_E,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().up(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_Q,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().down(deltaTime);
                                    }));

        controls.addKeyPressedCallback(
            GLFW_KEY_LEFT_SHIFT, new KeyCallback([this](float deltaTime) {
                this->getCamera().setTranslationSpeed(20);
            }));

        controls.addKeyReleasedCallback(
            GLFW_KEY_LEFT_SHIFT, new KeyCallback([this](float deltaTime) {
                this->getCamera().setTranslationSpeed(5);
            }));

        controls.addKeyPressedCallback(
            GLFW_KEY_SPACE, new KeyCallback([this](float deltaTime) {
                glm::vec3 cameraPos = this->getCamera().getPosition();
                glm::vec3 cameraFront = this->getCamera().getFront();
                this->spawnOrbitalBall(cameraPos, cameraFront, deltaTime);
            }));

        controls.addKeyPressedCallback(
            GLFW_KEY_O, new KeyCallback([this](float deltaTime) {
                this->spawnBall(this->getCamera().getPosition(),
                                this->getCamera().getFront(), 30, deltaTime);
            }));

        controls.addKeyPressedCallback(
            GLFW_KEY_H, new KeyCallback([this](float deltaTime) {
                this->getScene().toogleNode(this->ballSubScene);
            }));

        std::optional<Mesh> ballMeshOpt =
            FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");

        if (ballMeshOpt.has_value()) {
            this->ballMesh =
                SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                            ballMeshOpt.value());

            this->ballMesh.value().setAlbedo({0.5, 0.5, 0.5});
            this->ballMesh.value().setMetallic(0.5);
            this->ballMesh.value().setRoughness(0.4);
        } else {
            std::cout << "Mesh pas chargé correctement" << std::endl;
        }

        this->getScene().addLightToScene(
            Light(glm::vec3(0, 20, 0), glm::vec3(10000)));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        verlet.get()->update(deltaTime);

        for (size_t i = 0; i < this->ballIds.size(); i++) {
            this->getScene().setTransform(
                this->ballIds[i],
                translate(ECSManager::getManager()
                              .getComponentOfEntity<Positionable>(
                                  this->ballEntityIds[i])
                              .value()
                              .get()
                              .pos)
                    .scale(BALL_RADIUS));
        }
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

    SystemId write_csv_id = ECSManager::getManager().registerSystem(write_csv);
    ECSManager::getManager().registerComponentToSystem<Positionable>(
        write_csv_id);
    ECSManager::getManager().registerComponentToSystem<RigidBody>(write_csv_id);

    SystemId verletId = ECSManager::getManager().registerSystem(verlet);
    ECSManager::getManager().registerComponentToSystem<Positionable>(verletId);
    ECSManager::getManager().registerComponentToSystem<VerletBody>(verletId);

    Moteur engine(window, width, height);

    verlet->generateTerrain();

    for (auto &terrain : verlet->getTerrain().getSceneObjects()) {
        engine.getScene().addMesh(terrain);
    }

    engine.run();

    return 0;
}