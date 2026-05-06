#include "src/SceneObject.h"
#include "src/ecs/components/VerletBody.h"
#include <memory>
#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow *window;

#include "src/ContinuousLOD.h"
#include "src/FileLoader.cpp"
#include "src/GameEngine.h"
#include "src/Scene.h"
#include "src/Verlet.h"
#include "src/Write_csv.h"
#include "src/ecs/utils.h"

const float BALL_RADIUS = 0.2;

auto verlet = std::make_shared<Verlet>();
auto write_csv = std::make_shared<Write_CSV>();
auto continuousLOD = std::make_shared<ContinuousLOD>();

class Moteur : public GameEngine {
    std::optional<SceneObject> ballMesh;
    std::vector<NodeId> ballIds;
    std::vector<EntityId> ballEntityIds;

    std::optional<SceneObject> lodTestMesh;
    std::optional<SceneObject> lodTestMeshSphere;

    NodeId lodTestMeshId;
    NodeId lodTestMeshSphereId;

    EntityId lodTestMeshEntityId;
    EntityId lodTestMeshEntitySphereId;

    float distance = 0;

    void spawnBall(glm::vec3 pos, glm::vec3 direction, float speed,
                   float deltaTime) {
        NodeId ballId = this->getScene().addMesh(this->ballMesh.value());
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
        NodeId ballId = this->getScene().addMesh(this->ballMesh.value());
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
        this->getCamera().setTarget({0, 5, 0});
        glm::vec3 orbitalCenter = {0, 25, 0};
        float orbitalMass = 1000.0f;
        verlet->setOrbitalBody(orbitalCenter, orbitalMass);

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
            GLFW_KEY_O, new KeyCallback([this](float deltaTime) {
                this->spawnBall(this->getCamera().getPosition(),
                                this->getCamera().getFront(), 30, deltaTime);
            }));

        controls.addKeyDownCallback(GLFW_KEY_KP_ADD,
                                    new KeyCallback([this](float deltaTime) {
                                        this->distance += 25 * deltaTime;
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_KP_SUBTRACT,
                                    new KeyCallback([this](float deltaTime) {
                                        this->distance -= 25 * deltaTime;
                                    }));

        // std::optional<Mesh> ballMeshOpt =
        //     FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");

        // if (ballMeshOpt.has_value()) {
        //     this->ballMesh =
        //         SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
        //                     ballMeshOpt.value());

        //     this->ballMesh.value().setAlbedo({0.5, 0.5, 0.5});
        //     this->ballMesh.value().setMetallic(0.5);
        //     this->ballMesh.value().setRoughness(0.4);
        // } else {
        //     std::cout << "Mesh pas chargé correctement" << std::endl;
        // }

        std::optional<Mesh> lodMeshOpt =
            // FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");
            FileLoader::buildMeshFromOFF("assets/meshes/avion_n.off");

        std::optional<Mesh> lodMeshOptSphere =
            FileLoader::buildMeshFromOBJ("assets/meshes/sphere.obj");

        if (lodMeshOpt.has_value() && lodMeshOptSphere.has_value()) {
            this->lodTestMesh =
                SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                            lodMeshOpt.value());

            this->lodTestMeshSphere =
                SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                            lodMeshOptSphere.value());

            this->lodTestMesh.value().setAlbedo({0.5, 0.5, 0.5});
            this->lodTestMesh.value().setMetallic(0.5);
            this->lodTestMesh.value().setRoughness(0.4);

            this->lodTestMeshSphere.value().setAlbedo({0.5, 0.5, 0.5});
            this->lodTestMeshSphere.value().setMetallic(0.5);
            this->lodTestMeshSphere.value().setRoughness(0.4);

            // std::cout << "nombre de normales du mesh :"
            //           << lodMeshOpt.value().getNormals().size() << std::endl;
            // std::cout << "nombre de triangles du mesh :"
            //           << lodMeshOpt.value().getVertices().size() <<
            //           std::endl;

            this->lodTestMeshId =
                this->getScene().addMesh(this->lodTestMesh.value());

            this->lodTestMeshSphereId =
                this->getScene().addMesh(this->lodTestMeshSphere.value());

            this->lodTestMeshEntityId =
                ECSManager::getManager().generateEntityId();
            ECSManager::getManager().setComponentToEntity(
                Positionable(glm::vec3(0, 0, 0)), this->lodTestMeshEntityId);

            this->lodTestMeshEntitySphereId =
                ECSManager::getManager().generateEntityId();
            ECSManager::getManager().setComponentToEntity(
                Positionable(glm::vec3(1, 0, 0)),
                this->lodTestMeshEntitySphereId);

            ECSManager::getManager().setComponentToEntity(
                LOD(lodMeshOpt.value()), this->lodTestMeshEntityId);

            ECSManager::getManager().setComponentToEntity(
                LOD(lodMeshOptSphere.value()), this->lodTestMeshEntitySphereId);

            controls.addKeyPressedCallback(
                GLFW_KEY_V, new KeyCallback([this](float deltaTime) {
                    this->getScene()
                        .getMesh(this->lodTestMeshSphereId)
                        .value()
                        ->updateMeshData(
                            lodTestMesh.value().getMesh().getVertices(),
                            lodTestMesh.value().getMesh().getIndices(),
                            lodTestMesh.value().getMesh().getNormals(),
                            lodTestMesh.value().getMesh().getUvs());
                }));

            controls.addKeyDownCallback(
                GLFW_KEY_L, new KeyCallback([this](float deltaTime) {
                    this->getScene()
                        .getMesh(this->lodTestMeshId)
                        .value()
                        ->applyLOD();
                }));

        } else {
            std::cout << "LOD mesh pas chargé correctement" << std::endl;
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

        continuousLOD.get()->update(deltaTime, this->getCamera().getPosition(),
                                    distance);

        auto lodOpt = ECSManager::getManager().getComponentOfEntity<LOD>(
            this->lodTestMeshEntityId);

        if (lodOpt.has_value()) {
            // LOD &lod = lodOpt.value();
            // this->lodTestMesh.value().updateMeshData(
            //     lod.current_vertices, lod.current_indices,
            //     lod.current_normals, lod.current_uvs);
        }

        this->getScene().setTransform(this->lodTestMeshId,
                                      translate(glm::vec3(0, 5, 0)));

        this->getScene().setTransform(this->lodTestMeshSphereId,
                                      translate(glm::vec3(0, 10, 0)));
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

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    SystemId write_csv_id = ECSManager::getManager().registerSystem(write_csv);
    ECSManager::getManager().registerComponentToSystem<Positionable>(
        write_csv_id);
    ECSManager::getManager().registerComponentToSystem<RigidBody>(write_csv_id);

    SystemId verletId = ECSManager::getManager().registerSystem(verlet);
    ECSManager::getManager().registerComponentToSystem<Positionable>(verletId);
    ECSManager::getManager().registerComponentToSystem<VerletBody>(verletId);

    SystemId lodId = ECSManager::getManager().registerSystem(continuousLOD);
    ECSManager::getManager().registerComponentToSystem<LOD>(lodId);
    ECSManager::getManager().registerComponentToSystem<Positionable>(lodId);

    Moteur engine(window, width, height);

    engine.run();

    return 0;
}