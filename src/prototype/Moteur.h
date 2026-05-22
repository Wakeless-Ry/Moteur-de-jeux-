#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <optional>

#include "glm/detail/type_vec.hpp"
#include "src/AssetManager.h"
#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/Hud.h"
#include "src/VelocityIndicatorHud.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Collectible.h"
#include "src/ecs/components/Inventory.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/CollectibleSystem.h"
#include "src/ecs/systems/ContinuousLOD.h"
#include "src/ecs/systems/Gravity.h"
#include "src/ecs/systems/TransformPosition.h"
#include "src/ecs/systems/Verlet.h"
#include "src/ecs/utils.h"
#include "src/prototype/StellarSystem.h"

class Moteur : public GameEngine {

    SystemId verlet;

    EntityId characterEntityId;
    NodeId characterNodeId;

    std::optional<StellarSystem *> stellarSystem;

    float speed = 1.;

    Hud hud;
    int totalCollectibles = 10;
    VelocityIndicatorHud velHud;

    void init() override {
        this->initSystems();
        this->initScene();

        this->getSystemUpdater()->addSystem(std::make_shared<CollectibleSystem>(
            this->getScene(), this->characterEntityId));

        this->initInputs();
        this->getCamera().setTargetDistance(10);
        this->hud.init();
        this->velHud.init();
    }

    void initSystems() {
        this->verlet =
            this->getSystemUpdater()->addSystem(std::make_shared<Verlet>());

        this->getSystemUpdater()->addSystem(
            std::make_shared<TransformPosition>(this->getScene()));

        this->getSystemUpdater()->addSystem(std::make_shared<Gravity>());

        this->getSystemUpdater()->addSystem(std::make_shared<ContinuousLOD>(
            this->getScene(), this->getCamera()));

        this->stellarSystem = new StellarSystem(this->getScene());
    }

    void initScene() {
        this->getScene().addLightToScene(
            Light(glm::vec3(0, 0, 0), glm::vec3(10000)));

        std::shared_ptr<Mesh> ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        SceneObject character = SceneObject("shaders/PBR_vs.glsl",
                                            "shaders/PBR_fs.glsl", *ballMesh);

        character.addAlbedoMap(
            AssetManager::loadTexture("assets/textures/character.jpg"));
        character.setMetallic(0.5);
        character.setRoughness(0.4);

        this->characterEntityId = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(
            Noded(this->getScene().addMesh(character)), characterEntityId);
        glm::vec3 pos = {1000, 400, 0};
        ECSManager::setComponentToEntity(Positionable(pos), characterEntityId);
        ECSManager::setComponentToEntity(VerletBody(pos, {0, 0, 0}, 1, 0),
                                         characterEntityId);
        ECSManager::setComponentToEntity(Inventory{}, characterEntityId);
        this->stellarSystem.value()->setSystemAttraction(
            this->characterEntityId);

        ////////////////////////////////////////Collectible//////////////////////////////////

        this->totalCollectibles = 1;

        auto starMeshOpt = AssetManager::loadMesh("assets/meshes/star.obj");
        if (!starMeshOpt.has_value()) {
            return;
        }

        SceneObject starObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                               *starMeshOpt.value());

        starObject.setAlbedo(glm::vec3(1.0f, 0.9f, 0.2f));
        starObject.setMetallic(0.0f);
        starObject.setRoughness(0.6f);

        NodeId collectibleNodeId = this->getScene().addMesh(starObject);

        EntityId collectibleEntityId = ECSManager::generateEntityId();

        glm::vec3 collectiblePosition = glm::vec3(205.0f, 200.0f, 200.0f);

        float collectibleRadius = 1.0f;

        ECSManager::setComponentToEntity(Noded(collectibleNodeId),
                                         collectibleEntityId);
        ECSManager::setComponentToEntity(Positionable(collectiblePosition),
                                         collectibleEntityId);

        ECSManager::setComponentToEntity(VerletBody(collectiblePosition,
                                                    glm::vec3(0.0f),
                                                    collectibleRadius, true),
                                         collectibleEntityId);
        ECSManager::getComponentOfEntity<VerletBody>(collectibleEntityId)
            .value()
            .get()
            .isTrigger = true;
        ECSManager::setComponentToEntity(Collectible(1), collectibleEntityId);
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
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                body.acceleration += this->getCamera().getFront() * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_S, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                body.acceleration -= this->getCamera().getFront() * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_Q, new KeyCallback([this](float deltaTime) {
                this->getCamera().tilt(deltaTime, false);
            }));

        controls.addKeyDownCallback(GLFW_KEY_E,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().tilt(deltaTime, true);
                                    }));

        controls.addKeyPressedCallback(
            GLFW_KEY_SPACE, new KeyCallback([this](float deltaTime) {
                glm::vec3 &pos = ECSManager::getComponentOfEntity<Positionable>(
                                     this->characterEntityId)
                                     .value()
                                     .get()
                                     .pos;
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                pos += this->getCamera().getFront() * 10 * body.size;
                body.last_position = pos;
                body.acceleration = {};
            }));

        controls.addKeyDownCallback(GLFW_KEY_KP_ADD,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().getCloser(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_KP_SUBTRACT,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().getFurther(deltaTime);
                                    }));

        controls.addKeyPressedCallback(
            GLFW_KEY_C, new KeyCallback([this](float deltaTime) {
                Attracted &att = ECSManager::getComponentOfEntity<Attracted>(
                                     this->characterEntityId)
                                     .value();
                att.clear();
                att.addDirectionAttraction({-1, 0, 0}, 20);
            }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        if (this->stellarSystem.has_value()) {
            this->stellarSystem.value()->update(
                deltaTime, ECSManager::getComponentOfEntity<Positionable>(
                               this->characterEntityId)
                               .value()
                               .get()
                               .pos);
        }
    }

    void render(float deltaTime) override {
        std::optional<Positionable> pos =
            ECSManager::getComponentOfEntity<Positionable>(
                this->characterEntityId);

        if (pos.has_value()) {
            this->getCamera().setTarget(pos.value().pos);
        }

        hud.beginFrame((int)this->getCamera().getScreenWidth(),
                       (int)this->getCamera().getScreenHeight());

        int marginX = 40;
        int marginY = 40;
        int iconSize = 64;
        int spacing = 6;
        int maxWidth = (int)this->getCamera().getScreenWidth() - 2 * marginX;

        int collectedCount = 0;
        auto inventoryOpt = ECSManager::getComponentOfEntity<Inventory>(
            this->characterEntityId);

        if (inventoryOpt.has_value()) {
            collectedCount = inventoryOpt.value().get().collected;
        }

        hud.draw(collectedCount, totalCollectibles, marginX, marginY, iconSize,
                 spacing, maxWidth);

        hud.endFrame();
        velHud.render(this->getCamera(), this->characterEntityId, deltaTime);
    }

    void cleanUp() override { hud.cleanup(); }

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {}
};