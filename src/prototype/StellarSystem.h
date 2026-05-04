#include "src/AssetManager.h"
#include "src/GlobalScene.h"
#include "src/Transform.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"

class StellarSystem {
    GlobalScene &scene;

    NodeId stellarSystem;

    NodeId starParent;
    NodeId star;

    NodeId planetParent;
    NodeId planet;

    NodeId moonParent;
    NodeId moon;

    const float speed = 1;

    const float earthRevolutionRatio = 1;
    const float moonRevolutionRatio = 12.37;

    const float earthRotationRatio = 365;
    const float moonRotationRatio = 12.37;

    float sunAngle = 0;

    float planetRevolutionAngle = 0;
    float planetRotationAngle = 0;

    float moonRevolutionAngle = 0;
    float moonRotationAngle = 0;

  public:
    StellarSystem(GlobalScene &scene) : scene(scene) {
        this->stellarSystem = scene.addBasicNode();

        Mesh *ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        SceneObject starObject = SceneObject("shaders/PBR_vs.glsl",
                                             "shaders/PBR_fs.glsl", *ballMesh);

        starObject.setAlbedo({1, 1, 1});
        starObject.setMetallic(0.5);
        starObject.setRoughness(0.4);

        this->starParent =
            scene.addBasicNodeAsChild(this->stellarSystem).value();
        this->star = scene.addMeshAsChild(this->starParent, starObject).value();
        EntityId starEntityId = ECSManager::getManager().generateEntityId();
        ECSManager::getManager().setComponentToEntity(Noded(this->star),
                                                      starEntityId);
        ECSManager::getManager().setComponentToEntity(Positionable({0, 0, 0}),
                                                      starEntityId);

        SceneObject planetObject = SceneObject(
            "shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl", *ballMesh);

        planetObject.setAlbedo({1, 1, 1});
        planetObject.setMetallic(0.5);
        planetObject.setRoughness(0.4);

        this->planetParent =
            scene.addBasicNodeAsChild(this->starParent).value();
        this->planet =
            scene.addMeshAsChild(this->planetParent, planetObject).value();
        EntityId planetEntityId = ECSManager::getManager().generateEntityId();
        ECSManager::getManager().setComponentToEntity(Noded(this->planet),
                                                      planetEntityId);
        ECSManager::getManager().setComponentToEntity(Positionable({0, 0, 0}),
                                                      planetEntityId);

        SceneObject moonObject = SceneObject("shaders/PBR_vs.glsl",
                                             "shaders/PBR_fs.glsl", *ballMesh);

        moonObject.setAlbedo({1, 1, 1});
        moonObject.setMetallic(0.5);
        moonObject.setRoughness(0.4);

        this->moonParent =
            scene.addBasicNodeAsChild(this->planetParent).value();
        this->moon = scene.addMeshAsChild(this->moonParent, moonObject).value();
        EntityId moonEntityId = ECSManager::getManager().generateEntityId();
        ECSManager::getManager().setComponentToEntity(Noded(this->moon),
                                                      moonEntityId);
        ECSManager::getManager().setComponentToEntity(Positionable({0, 0, 0}),
                                                      moonEntityId);
    }

    void update(float deltaTime) {
        float deltaSpeed = deltaTime * speed;

        this->sunAngle += 0.2 * deltaSpeed;
        this->scene.setTransform(this->star,
                                 rotationY(this->sunAngle).scale(10 * 1.5));

        this->planetRevolutionAngle += earthRevolutionRatio * deltaSpeed;
        this->scene.setTransform(
            this->planetParent,
            rotationY(this->planetRevolutionAngle).translate(100, 0, 0));
        this->planetRotationAngle += earthRotationRatio * deltaSpeed;
        this->scene.setTransform(
            this->planet,
            rotationX(23).rotationY(this->planetRotationAngle).scale(10 * 0.6));

        this->moonRevolutionAngle += moonRevolutionRatio * deltaSpeed;
        this->scene.setTransform(
            this->moonParent,
            rotationY(this->moonRevolutionAngle).translate(20, 0, 0));
        this->moonRotationAngle += moonRotationRatio * deltaSpeed;
        this->scene.setTransform(
            this->moon, rotationY(this->planetRotationAngle).scale(10 * 0.25));
    }
};