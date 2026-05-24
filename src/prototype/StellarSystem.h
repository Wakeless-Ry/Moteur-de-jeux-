#include <memory>
#include <optional>

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include <glm/ext.hpp>

#include "src/AssetManager.h"
#include "src/GlobalScene.h"
#include "src/Transform.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/utils.h"

class StellarSystem {
    GlobalScene &scene;

    NodeId stellarSystem;

    NodeId starParent;
    NodeId star;
    NodeId starPipe;
    EntityId starEntityId;

    NodeId planetParent;
    NodeId planet;
    NodeId planetPipe;
    EntityId planetEntityId;

    NodeId moonParent;
    NodeId moon;
    NodeId moonPipe;
    EntityId moonEntityId;

    const float starMass = 500000;
    const float starSize = 150;

    const float planetMass = 100000;
    const float planetSize = 60;

    const float moonMass = 50000;
    const float moonSize = 25;

    const float speed = 0;

    const float starRotationRatio = 0.2;

    const float planetRevolutionRatio = 1;
    const float moonRevolutionRatio = 12.37;

    const float planetRotationRatio = 365;
    const float moonRotationRatio = 12.37;

    float starRotationAngle = 0;

    float planetRevolutionAngle = 0;
    float planetRotationAngle = 0;

    float moonRevolutionAngle = 0;
    float moonRotationAngle = 0;

    void registerCelestialObject(const char *texturePath, NodeId &parent,
                                 NodeId &nodeParent, NodeId &node, NodeId &pipe,
                                 EntityId &entity, float size) {

        std::shared_ptr<Mesh> ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        SceneObject celestialObject = SceneObject(
            "shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl", *ballMesh);

        // celestialObject.setAlbedo({1, 1, 1});
        celestialObject.addAlbedoMap(AssetManager::loadTexture(texturePath));
        celestialObject.setMetallic(0.5);
        celestialObject.setRoughness(0.4);

        nodeParent = scene.addBasicNodeAsChild(parent).value();
        node = scene.addMeshAsChild(nodeParent, celestialObject).value();
        entity = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(Noded(node), entity);

        glm::vec3 pos = {0, 0, 0};

        ECSManager::setComponentToEntity(Positionable(pos, false), entity);
        ECSManager::setComponentToEntity(VerletBody(pos, {}, size, true),
                                         entity);

        std::shared_ptr<Mesh> pipeMesh =
            AssetManager::loadMesh("assets/meshes/pipe.obj").value();
        SceneObject pipeObject = SceneObject("shaders/PBR_vs.glsl",
                                             "shaders/PBR_fs.glsl", *pipeMesh);

        pipeObject.setAlbedo({0, 0.8, 0});
        pipeObject.setMetallic(0.3);
        pipeObject.setRoughness(0.8);

        pipe = scene.addMeshAsChild(node, pipeObject).value();
        EntityId pipeId = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(Noded(pipe), pipeId);
    }

  public:
    StellarSystem(GlobalScene &scene) : scene(scene) {
        this->stellarSystem = scene.addBasicNode();

        this->registerCelestialObject(
            "assets/textures/sun.jpg", this->stellarSystem, this->starParent,
            this->star, this->starPipe, this->starEntityId, starSize);

        this->registerCelestialObject(
            "assets/textures/earth.jpg", this->starParent, this->planetParent,
            this->planet, this->planetPipe, this->planetEntityId, planetSize);

        this->registerCelestialObject(
            "assets/textures/moon.jpg", this->planetParent, this->moonParent,
            this->moon, this->moonPipe, this->moonEntityId, moonSize);
    }

    void update(float deltaTime, const glm::vec3 pos) {
        float deltaSpeed = deltaTime * speed;

        glm::vec3 starPos =
            ECSManager::getComponentOfEntity<Positionable>(this->starEntityId)
                .value()
                .get()
                .pos;
        glm::vec3 planetPos =
            ECSManager::getComponentOfEntity<Positionable>(this->planetEntityId)
                .value()
                .get()
                .pos;
        glm::vec3 moonPos =
            ECSManager::getComponentOfEntity<Positionable>(this->moonEntityId)
                .value()
                .get()
                .pos;

        if (glm::distance(starPos, pos) > this->starSize * 1.5) {
            this->starRotationAngle += starRotationRatio * deltaSpeed;
        }

        this->scene.setTransform(
            this->star, rotationY(this->starRotationAngle).scale(starSize));
        this->scene.setTransform(
            this->starPipe,
            translate(1, 0, 0).rotationZ(90).scale(1.f / starSize));

        if (glm::distance(planetPos, pos) > this->planetSize * 1.5) {
            this->planetRevolutionAngle += planetRevolutionRatio * deltaSpeed;
            this->planetRotationAngle += planetRotationRatio * deltaSpeed;
        }

        this->scene.setTransform(
            this->planetParent,
            rotationY(this->planetRevolutionAngle).translate(1000, 0, 0));
        this->scene.setTransform(this->planet,
                                 rotationX(23)
                                     .rotationX(90)
                                     .rotationZ(this->planetRotationAngle)
                                     .scale(planetSize));

        this->scene.setTransform(
            this->planetPipe,
            translate(0.f, 0.f, -1.f).rotationX(90.f).scale(4.f / planetSize));

        if (glm::distance(moonPos, pos) > this->moonSize * 1.5) {
            this->moonRevolutionAngle += moonRevolutionRatio * deltaSpeed;
            this->moonRotationAngle += moonRotationRatio * deltaSpeed;
        }

        this->scene.setTransform(
            this->moonParent,
            rotationY(this->moonRevolutionAngle).translate(200, 0, 0));
        this->scene.setTransform(this->moon, scale(moonSize));
        this->scene.setTransform(
            this->moonPipe,
            translate(1, 0, 0).rotationZ(90).scale(1.f / moonSize));

        glm::vec3 posPlanet =
            ECSManager::getComponentOfEntity<Positionable>(this->planetEntityId)
                .value()
                .get()
                .pos;
        glm::vec3 posMoon =
            ECSManager::getComponentOfEntity<Positionable>(this->moonEntityId)
                .value()
                .get()
                .pos;
    }

    bool isNearPipe(const glm::vec3 &playerPos, float triggerDist) {
        const float triggerDist2 = triggerDist * triggerDist;

        auto nearPipe = [&](NodeId id) {
            auto opt = this->scene.getTransform(id);
            if (!opt.has_value()) {
                return false;
            }

            const glm::vec3 pipePos = opt.value().getPosition();
            const glm::vec3 d = pipePos - playerPos;
            const float dist2 = glm::dot(d, d);
            return dist2 <= triggerDist2;
        };

        return nearPipe(this->starPipe) || nearPipe(this->planetPipe) ||
               nearPipe(this->moonPipe);
    }

    bool getNearestPipePos(const glm::vec3 &playerPos, float triggerDist,
                           glm::vec3 &outPipePos) {
        if (triggerDist <= 0.0f)
            return false;

        float bestDist2 = triggerDist * triggerDist;
        bool found = false;

        auto consider = [&](NodeId pipeNode) {
            auto opt = scene.getTransform(pipeNode);
            if (!opt.has_value())
                return;

            const glm::vec3 pipePos = opt.value().getPosition();
            const glm::vec3 d = pipePos - playerPos;
            const float dist2 = glm::dot(d, d);

            if (dist2 <= bestDist2) {
                bestDist2 = dist2;
                outPipePos = pipePos;
                found = true;
            }
        };

        consider(starPipe);
        consider(planetPipe);
        consider(moonPipe);

        return found;
    }

    void setSystemAttraction(EntityId id) {
        if (!ECSManager::getComponentOfEntity<Attracted>(id).has_value()) {
            ECSManager::setComponentToEntity(Attracted(), id);
        }

        Attracted &attracted =
            ECSManager::getComponentOfEntity<Attracted>(id).value();
        attracted.addBodyAttraction(this->starEntityId, AttractionMode::INWARD,
                                    starMass);
        attracted.addBodyAttraction(this->planetEntityId,
                                    AttractionMode::INWARD, planetMass / 10);
        attracted.addBodyAttraction(this->moonEntityId, AttractionMode::INWARD,
                                    moonMass);
    }
};