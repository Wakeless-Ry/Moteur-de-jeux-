#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Observer.h"

using namespace std;
using namespace glm;

using uint = unsigned int;

enum CameraMode { LOOK_AT, FRONT, HOVER, ROTATE, Count };

inline CameraMode &operator++(CameraMode &dir) {
    dir = static_cast<CameraMode>((static_cast<int>(dir) + 1) %
                                  static_cast<int>(CameraMode::Count));
    return dir;
}

const static vec3 VEC_UP(0.f, 1.f, 0.f);
const static vec3 VEC_FRONT(0.f, 0.f, 1.f);
const static vec3 VEC_RIGHT(1.f, 0.f, 0.f);

static float clipAngle180(float _angle);

class Camera : public Observer<vec3> {
    uint screenWidth;
    uint screenHeight;

    float fov = 45;
    float zNear = .1;
    float zFar = 100;

    vec3 position = vec3(0, 0, 0);
    // vec3 eulerAngle = vec3(45, 45, 0);
    vec3 eulerAngle = vec3(0, 0, 0);
    quat rotation = quat();

    mat4 viewMatrix = mat4();
    mat4 projectionMatrix = mat4();

    vec3 target = vec3(0, 0, 0);
    float targetDistance = 3.;

    float translationSpeed = 2.5;
    float rotationSpeed = 100;

    CameraMode mode = LOOK_AT;

    vec3 projectVectorOnPlan(vec3 toProject, vec3 normal);

  public:
    Camera(uint screenWidth, uint screenHeight);

    void update(const vec3 &pos) override;

    uint getScreenWidth() const;
    void setScreenWidth(uint newWidth);

    uint getScreenHeight() const;
    void setScreenHeight(uint newHeight);

    float getFov() const;
    void setFov(float newFov);

    float getZNear() const;
    void setZNear(float newZNear);

    float getZFar() const;
    void setZFar(float newZFar);

    vec3 getPosition() const;
    void setPosition(vec3 newPosition);

    vec3 getEulerAngle() const;
    void setEulerAngle(vec3 newEulerAngle);

    vec3 getTarget() const;
    void setTarget(vec3 newTarget);

    float getTargetDistance() const;
    void setTargetDistance(float newTargetDistance);

    float getTranslationSpeed() const;
    void setTranslationSpeed(float newTranslationSpeed);

    float getRotationSpeed() const;
    void setRotationSpeed(float newRotationSpeed);

    vec3 getFront() const;

    void update(float deltaTime);
    void update(float deltaTime, vec3 newTarget);

    mat4 getView() const;
    mat4 getProjection() const;

    void rotateWithMouse(float deltaX, float deltaY);

    void forward(float deltaTime);
    void backward(float deltaTime);
    void left(float deltaTime);
    void right(float deltaTime);
    void increaseRotationSpeed(float deltaTime);
    void decreaseRotationSpeed(float deltaTime);
    void up(float deltaTime);
    void down(float deltaTime);

    CameraMode changeMode();
};

#endif // CAMERA