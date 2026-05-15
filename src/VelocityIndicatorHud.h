#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "src/AssetManager.h"
#include "src/Camera.h"
#include "src/Light.hpp"
#include "src/SceneObject.h"
#include "src/Transform.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <optional>

/*
TODO:
-Les flèches devraient se déplacer relatif en fonction du monde comme l'affichge
des axes comme en blender La sphère est la pour illustrer l'objet
*/

class VelocityIndicatorHud {
  public:
    const char *sphereMeshPath = "assets/meshes/big_sphere.obj";
    const char *arrowMeshPath = "assets/meshes/fleche.obj";

    const char *vsPath = "shaders/PBR_vs.glsl";
    const char *fsPath = "shaders/PBR_fs.glsl";

    int hudSizePx = 220;
    int hudMarginPx = 20;

    float maxVelocity = 30.f;
    float minArrowLen = 0.05f;
    float maxArrowLen = 0.5f;
    float arrowThickness = 0.10f;
    float sphereScale = 0.60f;

    float smoothingK = 12.0f;

    int hudYawSteps = 8;
    int hudPitchSteps = 6;

    float widgetYawDeg = 35.0f;
    float widgetPitchDeg = -25.0f;

    bool invertRightAxis = true;
    bool invertFrontAxis = true;
    bool invertUpAxis = false;

    void init() {
        if (initialized)
            return;

        auto sphereMeshOpt = AssetManager::loadMesh(sphereMeshPath);
        if (!sphereMeshOpt.has_value()) {
            return;
        }

        sphere.emplace(vsPath, fsPath, *sphereMeshOpt.value());

        auto arrowMeshOpt = AssetManager::loadMesh(arrowMeshPath);
        if (!arrowMeshOpt.has_value()) {
            return;
        }
        arrowRight.emplace(vsPath, fsPath, *arrowMeshOpt.value());
        arrowFront.emplace(vsPath, fsPath, *arrowMeshOpt.value());
        arrowUp.emplace(vsPath, fsPath, *arrowMeshOpt.value());

        sphere->setAlbedo(glm::vec3(0.08f, 0.08f, 0.10f));
        sphere->setMetallic(0.0f);
        sphere->setRoughness(0.9f);

        arrowRight->setMetallic(0.0f);
        arrowRight->setRoughness(0.6f);
        arrowFront->setMetallic(0.0f);
        arrowFront->setRoughness(0.6f);
        arrowUp->setMetallic(0.0f);
        arrowUp->setRoughness(0.6f);

        hudCam = Camera(hudSizePx, hudSizePx);
        hudCam.setTarget(glm::vec3(0.0f));
        hudCam.setTargetDistance(3.0f);
        hudCam.setZNear(0.01f);
        hudCam.setZFar(50.0f);

        for (int i = 0; i < hudYawSteps; ++i) {
            hudCam.rotateWithMouse(-50.0f, 0.0f);
        }
        for (int i = 0; i < hudPitchSteps; ++i) {
            hudCam.rotateWithMouse(0.0f, -50.0f);
        }

        lights.clear();
        lights.push_back(
            Light(glm::vec3(-1.5f, -1.5f, -2.0f), glm::vec3(30.0f)));

        initialized = true;
    }

    void render(const Camera &mainCam, EntityId trackedEntity,
                float deltaTime) {
        if (!initialized)
            return;
        if (!sphere.has_value() || !arrowRight.has_value() ||
            !arrowFront.has_value() || !arrowUp.has_value())
            return;

        glm::vec3 velocityWorld(0.0f);
        auto posOpt =
            ECSManager::getComponentOfEntity<Positionable>(trackedEntity);
        auto verletOpt =
            ECSManager::getComponentOfEntity<VerletBody>(trackedEntity);

        if (posOpt.has_value() && verletOpt.has_value() &&
            deltaTime > 0.00001f) {
            const glm::vec3 currentPosition = posOpt.value().get().pos;
            const glm::vec3 previousPosition =
                verletOpt.value().get().last_position;
            velocityWorld = (currentPosition - previousPosition) / deltaTime;
        }

        const glm::vec3 cameraRight = glm::normalize(mainCam.getRight());
        const glm::vec3 cameraFront = glm::normalize(mainCam.getFront());
        const glm::vec3 cameraUp = glm::normalize(mainCam.getUp());

        float velocityRight = glm::dot(velocityWorld, cameraRight);
        if (invertRightAxis) {
            velocityRight = -velocityRight;
        }

        float velocityFront = glm::dot(velocityWorld, cameraFront);
        if (invertFrontAxis) {
            velocityFront = -velocityFront;
        }

        float velocityUp = glm::dot(velocityWorld, cameraUp);
        if (invertUpAxis) {
            velocityUp = -velocityUp;
        }

        auto normalizeMagnitude = [&](float componentValue) {
            return std::clamp(std::abs(componentValue) / maxVelocity, 0.0f,
                              1.0f);
        };

        float normMagnRight = normalizeMagnitude(velocityRight);
        float normMagnFront = normalizeMagnitude(velocityFront);
        float normMagnUp = normalizeMagnitude(velocityUp);

        if (smoothingK > 0.0f && deltaTime > 0.0f) {
            float alpha = 1.0f - std::exp(-smoothingK * deltaTime);
            smoothedMagnRight =
                glm::mix(smoothedMagnRight, normMagnRight, alpha);
            smoothedMagnFront =
                glm::mix(smoothedMagnFront, normMagnFront, alpha);
            smoothedMagnUp = glm::mix(smoothedMagnUp, normMagnUp, alpha);
        } else {
            smoothedMagnRight = normMagnRight;
            smoothedMagnFront = normMagnFront;
            smoothedMagnUp = normMagnUp;
        }

        GLint prevViewport[4];
        glGetIntegerv(GL_VIEWPORT, prevViewport);

        int viewportSizePx = hudSizePx;
        int viewportLeftPx = hudMarginPx;
        int viewportBottomPx = hudMarginPx;

        glViewport(viewportLeftPx, viewportBottomPx, viewportSizePx,
                   viewportSizePx);

        glEnable(GL_SCISSOR_TEST);
        glScissor(viewportLeftPx, viewportBottomPx, viewportSizePx,
                  viewportSizePx);

        glClear(GL_DEPTH_BUFFER_BIT);

        hudCam.setScreenWidth((uint)viewportSizePx);
        hudCam.setScreenHeight((uint)viewportSizePx);
        hudCam.update(deltaTime);

        Transform base = makeWidgetBaseTransform();

        {
            Transform sphereTransform;
            sphereTransform.transform(base);
            sphereTransform.scale(sphereScale);
            sphere->setTransform(sphereTransform);

            glEnable(GL_DEPTH_TEST);
            sphere->draw(hudCam, lights);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        drawAxisArrow(*arrowRight, smoothedMagnRight, velocityRight,
                      Axis::Right, base);
        drawAxisArrow(*arrowFront, smoothedMagnFront, velocityFront,
                      Axis::Front, base);
        drawAxisArrow(*arrowUp, smoothedMagnUp, velocityUp, Axis::Up, base);

        glDepthMask(GL_TRUE);

        glDisable(GL_SCISSOR_TEST);

        glViewport(prevViewport[0], prevViewport[1], prevViewport[2],
                   prevViewport[3]);
    }

  private:
    enum class Axis { Right, Front, Up };

    bool initialized = false;

    std::optional<SceneObject> sphere;
    std::optional<SceneObject> arrowRight, arrowFront, arrowUp;

    Camera hudCam{1, 1};
    std::vector<Light> lights;

    float smoothedMagnRight = 0.0f;
    float smoothedMagnFront = 0.0f;
    float smoothedMagnUp = 0.0f;

    float lerpLen(float a01) const {
        return minArrowLen + a01 * (maxArrowLen - minArrowLen);
    }

    Transform makeWidgetBaseTransform() const {
        Transform base;
        base.rotationY(widgetYawDeg);
        base.rotationX(widgetPitchDeg);
        return base;
    }

    void drawAxisArrow(SceneObject &arrow, float mag01Smoothed,
                       float signedComponent, Axis axis,
                       const Transform &base) {
        if (mag01Smoothed <= 0.0f)
            return;

        float arrowLength = lerpLen(mag01Smoothed);
        float sign = (signedComponent >= 0.0f) ? 1.0f : -1.0f;

        if (axis == Axis::Right)
            arrow.setAlbedo(glm::vec3(1.0f, 0.35f, 0.35f));
        if (axis == Axis::Front)
            arrow.setAlbedo(glm::vec3(0.35f, 1.0f, 0.35f));
        if (axis == Axis::Up)
            arrow.setAlbedo(glm::vec3(0.35f, 0.65f, 1.0f));

        float baseOffset = sphereScale * 0.55f;

        Transform arrowTransf;
        arrowTransf.transform(base);

        glm::vec3 axisDirection(0.0f);
        switch (axis) {
        case Axis::Right:
            axisDirection = glm::vec3(sign, 0, 0);
            break;
        case Axis::Front:
            axisDirection = glm::vec3(0, 0, sign);
            break;
        case Axis::Up:
            axisDirection = glm::vec3(0, sign, 0);
            break;
        }

        arrowTransf.translate(axisDirection * baseOffset);

        applyAxisRotationFromPlusY(arrowTransf, axisDirection);

        arrowTransf.scale(arrowThickness, arrowLength, arrowThickness);

        arrow.setTransform(arrowTransf);
        arrow.draw(hudCam, lights);
    }

    void applyAxisRotationFromPlusY(Transform &transform,
                                    const glm::vec3 &axisDirection) {
        if (axisDirection.y > 0.5f)
            return;
        if (axisDirection.y < -0.5f) {
            transform.rotationZ(180.0f);
            return;
        }

        if (axisDirection.x > 0.5f) {
            transform.rotationZ(-90.0f);
            return;
        }
        if (axisDirection.x < -0.5f) {
            transform.rotationZ(90.0f);
            return;
        }

        if (axisDirection.z > 0.5f) {
            transform.rotationX(-90.0f);
            return;
        }
        if (axisDirection.z < -0.5f) {
            transform.rotationX(90.0f);
            return;
        }
    }
};