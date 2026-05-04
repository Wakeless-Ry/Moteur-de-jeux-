#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <optional>

#include "glm/detail/type_vec.hpp"
#include "src/AssetManager.h"
#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/TransformPosition.h"
#include "src/ecs/systems/Verlet.h"
#include "src/ecs/utils.h"
#include "src/prototype/StellarSystem.h"

class Moteur : public GameEngine {

    SystemId verlet;

    EntityId characterEntityId;

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

        this->getSystemUpdater()->addSystem(
            std::make_shared<TransformPosition>(this->getScene()));
    }

    void initScene() {
        this->getScene().addLightToScene(
            Light(glm::vec3(0, 0, 0), glm::vec3(10000)));

        Mesh *ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        SceneObject character = SceneObject("shaders/PBR_vs.glsl",
                                            "shaders/PBR_fs.glsl", *ballMesh);

        character.addAlbedoMap(
            AssetManager::loadTexture("assets/textures/character.jpg"));
        character.setMetallic(0.5);
        character.setRoughness(0.4);

        this->characterEntityId = ECSManager::getManager().generateEntityId();
        ECSManager::getManager().setComponentToEntity(
            Noded(this->getScene().addMesh(character)), characterEntityId);
        glm::vec3 pos = {0, 0, 0};
        ECSManager::getManager().setComponentToEntity(Positionable(pos),
                                                      characterEntityId);
        ECSManager::getManager().setComponentToEntity(
            VerletBody(pos, {0, 0, 0}, 1, 0), characterEntityId);
    }

    void initInputs() {
        Controls &controls = this->getControls();

        controls.addKeyPressedCallback(
            GLFW_KEY_ESCAPE,
            new KeyCallback([this](float deltaTime) { this->stopRunning(); }));

        controls.addMouseDeltaCallback(
            new MouseMoveCallback([this](float dx, float dy) {
                this->getCamera().rotateWithMouse(dx, dy);
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_W, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getManager()
                                       .getComponentOfEntity<VerletBody>(
                                           this->characterEntityId)
                                       .value();

                body.acceleration += this->getCamera().getFront() * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_S, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getManager()
                                       .getComponentOfEntity<VerletBody>(
                                           this->characterEntityId)
                                       .value();

                body.acceleration -= this->getCamera().getFront() * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_SPACE, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getManager()
                                       .getComponentOfEntity<VerletBody>(
                                           this->characterEntityId)
                                       .value();

                body.acceleration += this->getCamera().getUp() * 10;
            }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        if (this->stellarSystem.has_value()) {
            this->stellarSystem.value()->update(deltaTime);
        }
    }

    void render(float deltaTime) override {
        std::optional<Positionable> pos =
            ECSManager::getManager().getComponentOfEntity<Positionable>(
                this->characterEntityId);

        if (pos.has_value()) {
            this->getCamera().setTarget(pos.value().pos);
        }
    }

    void cleanUp() override {}

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {}
};