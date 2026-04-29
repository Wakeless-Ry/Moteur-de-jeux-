#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "src/AssetManager.h"
#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/systems/TransformPosition.h"
#include "src/ecs/systems/Verlet.h"
#include "src/ecs/utils.h"

class Moteur : public GameEngine {

    std::shared_ptr<Verlet> verletSystem;
    std::shared_ptr<TransformPosition> transformPosition;

    void init() override {
        this->initSystems();
        this->initScene();
        this->initInputs();
        this->getCamera().changeMode();
    }

    void initSystems() {
        SystemId verletId =
            ECSManager::getManager().registerSystem(this->verletSystem);
        ECSManager::getManager().registerComponentToSystem<Positionable>(
            verletId);
        ECSManager::getManager().registerComponentToSystem<VerletBody>(
            verletId);

        SystemId transPosId =
            ECSManager::getManager().registerSystem(this->transformPosition);
        ECSManager::getManager().registerComponentToSystem<Positionable>(
            transPosId);
        ECSManager::getManager().registerComponentToSystem<Noded>(transPosId);
    }

    void initScene() {
        this->getScene().addLightToScene(
            Light(glm::vec3(0, 20, 0), glm::vec3(10000)));

        std::optional<Mesh *> ballMeshOpt =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj");

        if (ballMeshOpt.has_value()) {
            SceneObject ballMesh =
                SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                            *ballMeshOpt.value());

            ballMesh.setAlbedo({0.5, 0.5, 0.5});
            ballMesh.setMetallic(0.5);
            ballMesh.setRoughness(0.4);

            EntityId ballEntityId = ECSManager::getManager().generateEntityId();
            ECSManager::getManager().setComponentToEntity(
                Noded(this->getScene().addMesh(ballMesh)), ballEntityId);
            ECSManager::getManager().setComponentToEntity(
                Positionable({0, 0, 0}), ballEntityId);
            ECSManager::getManager().setComponentToEntity(
                VerletBody({0, 0, 0}, {0, 0, 0}, 1, 0), ballEntityId);
        }
    }

    void initInputs() {
        Controls &controls = this->getControls();

        controls.addKeyPressedCallback(
            GLFW_KEY_ESCAPE,
            new KeyCallback([this](float deltaTime) { this->stopRunning(); }));

        controls.addKeyPressedCallback(
            GLFW_KEY_M, new KeyCallback([this](float deltaTime) {
                CameraMode mode = this->getCamera().changeMode();
            }));

        controls.addMouseDeltaCallback(
            new MouseMoveCallback([this](float dx, float dy) {
                this->getCamera().rotateWithMouse(dx, dy);
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
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {}

    void render(float deltaTime) override {
        this->verletSystem->update(deltaTime);
        this->transformPosition->update(deltaTime);
    }

    void cleanUp() override {}

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {
        this->verletSystem = std::make_shared<Verlet>();
        this->transformPosition =
            std::make_shared<TransformPosition>(this->getScene());
    }
};