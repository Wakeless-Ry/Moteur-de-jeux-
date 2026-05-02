#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/ecs/systems/TransformPosition.h"
#include "src/ecs/systems/Verlet.h"
#include "src/ecs/utils.h"
#include "src/prototype/StellarSystem.h"

class Moteur : public GameEngine {

    SystemId verlet;

    std::optional<StellarSystem *> stellarSystem;

    float speed = 1.;

    void init() override {
        this->initSystems();
        this->initScene();
        this->initInputs();
        this->getCamera().setTargetDistance(10);
        this->stellarSystem = new StellarSystem(this->getScene());
    }

    void initSystems() {
        this->verlet =
            this->getSystemUpdater()->addSystem(std::make_shared<Verlet>());
        this->getSystemUpdater()->disable(this->verlet);

        this->getSystemUpdater()->addSystem(
            std::make_shared<TransformPosition>(this->getScene()));
    }

    void initScene() {
        this->getScene().addLightToScene(
            Light(glm::vec3(0, 50, 0), glm::vec3(10000)));
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

        controls.addKeyPressedCallback(
            GLFW_KEY_SPACE, new KeyCallback([this](float deltaTime) {
                this->getSystemUpdater()->toggle(this->verlet);
            }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        if (this->stellarSystem.has_value()) {
            this->stellarSystem.value()->update(deltaTime);
        }
    }

    void render(float deltaTime) override {}

    void cleanUp() override {}

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {}
};