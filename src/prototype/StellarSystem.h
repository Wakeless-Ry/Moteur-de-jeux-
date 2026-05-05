#include "glm/detail/type_vec.hpp"
#include "src/AssetManager.h"
#include "src/GlobalScene.h"
#include "src/Transform.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/utils.h"
#include <optional>

class StellarSystem {
    GlobalScene &scene;

    NodeId stellarSystem;

    NodeId starParent;
    NodeId star;
    EntityId starEntityId;

    NodeId planetParent;
    NodeId planet;
    EntityId planetEntityId;

    NodeId moonParent;
    NodeId moon;
    EntityId moonEntityId;

    const float starMass = 1000;
    const float starSize = 15;

    const float planetMass = 100;
    const float planetSize = 6;

    const float moonMass = 10;
    const float moonSize = 2.5;

    const float speed = 1;

    const float earthRevolutionRatio = 1;
    const float moonRevolutionRatio = 12.37;

    const float earthRotationRatio = 365;
    const float moonRotationRatio = 12.37;

    float starAngle = 0;

    float planetRevolutionAngle = 0;
    float planetRotationAngle = 0;

    float moonRevolutionAngle = 0;
    float moonRotationAngle = 0;

    void registerCelestialObject(Mesh &ballMesh, NodeId &parent,
                                 NodeId &nodeParent, NodeId &node,
                                 EntityId &entity, float size) {
        SceneObject celestialObject =
            SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl", ballMesh);

        celestialObject.setAlbedo({1, 1, 1});
        celestialObject.setMetallic(0.5);
        celestialObject.setRoughness(0.4);

        nodeParent = scene.addBasicNodeAsChild(parent).value();
        node = scene.addMeshAsChild(nodeParent, celestialObject).value();
        entity = ECSManager::getManager().generateEntityId();
        ECSManager::getManager().setComponentToEntity(Noded(node), entity);

        glm::vec3 pos = {0, 0, 0};

        ECSManager::getManager().setComponentToEntity(Positionable(pos, false),
                                                      entity);
        ECSManager::getManager().setComponentToEntity(
            VerletBody(pos, {}, size, true), entity);
    }

  public:
    StellarSystem(GlobalScene &scene) : scene(scene) {
        this->stellarSystem = scene.addBasicNode();

        Mesh *ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        this->registerCelestialObject(*ballMesh, this->stellarSystem,
                                      this->starParent, this->star,
                                      this->starEntityId, starSize);

        this->registerCelestialObject(*ballMesh, this->starParent,
                                      this->planetParent, this->planet,
                                      this->planetEntityId, planetSize);

        this->registerCelestialObject(*ballMesh, this->planetParent,
                                      this->moonParent, this->moon,
                                      this->moonEntityId, moonSize);
    }

    void update(float deltaTime) {
        float deltaSpeed = deltaTime * speed;

        this->starAngle += 0.2 * deltaSpeed;
        this->scene.setTransform(this->star,
                                 rotationY(this->starAngle).scale(starSize));

        this->planetRevolutionAngle += earthRevolutionRatio * deltaSpeed;
        this->scene.setTransform(
            this->planetParent,
            rotationY(this->planetRevolutionAngle).translate(100, 0, 0));
        this->planetRotationAngle += earthRotationRatio * deltaSpeed;
        this->scene.setTransform(this->planet,
                                 rotationX(23)
                                     .rotationY(this->planetRotationAngle)
                                     .scale(planetSize));

        this->moonRevolutionAngle += moonRevolutionRatio * deltaSpeed;
        this->scene.setTransform(
            this->moonParent,
            rotationY(this->moonRevolutionAngle).translate(20, 0, 0));
        this->moonRotationAngle += moonRotationRatio * deltaSpeed;
        this->scene.setTransform(
            this->moon, rotationY(this->planetRotationAngle).scale(moonSize));
    }

    void setSystemAttraction(EntityId id) {
        if (!ECSManager::getManager()
                 .getComponentOfEntity<Attracted>(id)
                 .has_value()) {
            ECSManager::getManager().setComponentToEntity(Attracted(), id);
        }

        Attracted &attracted = ECSManager::getManager()
                                   .getComponentOfEntity<Attracted>(id)
                                   .value();
        attracted.addAttraction(this->starEntityId, AttractionMode::INWARD,
                                starMass);
        attracted.addAttraction(this->planetEntityId, AttractionMode::INWARD,
                                planetMass);
        attracted.addAttraction(this->moonEntityId, AttractionMode::INWARD,
                                moonMass);
    }
};