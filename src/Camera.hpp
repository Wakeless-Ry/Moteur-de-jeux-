#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

using uint = unsigned int;

enum CameraMode {
    LOOK_AT,
    FRONT,
    HOVER,
    Count
};

inline CameraMode& operator++(CameraMode& dir) {
    dir = static_cast<CameraMode>((static_cast<int>(dir) + 1) % static_cast<int>(CameraMode::Count));
    return dir;
}

const static vec3 VEC_UP(0.f, 1.f, 0.f);
const static vec3 VEC_FRONT(0.f, 0.f, 1.f);
const static vec3 VEC_RIGHT(1.f, 0.f, 0.f);

static float clipAngle180(float _angle) {
    while (_angle >= 180.f || _angle < -180.f) {
        _angle += (_angle > 180.f) ? -360.f : 360.f;
    }
    return _angle;
}

class Camera {
    uint screenWidth;
    uint screenHeight;

    float fov = 45;
    float zNear = .1;
    float zFar = 100;

    vec3 position = vec3(0, 0, 0);
    vec3 eulerAngle = vec3(45, 45, 0);
    quat rotation = quat();

    mat4 viewMatrix = mat4();
    mat4 projectionMatrix = mat4();

    vec3 target = vec3(0, 0, 0);
    float targetDistance = 3.;

    float translationSpeed = 2.5;
    float rotationSpeed = 100;

    CameraMode mode = LOOK_AT;

    vec3 projectVectorOnPlan(vec3 toProject, vec3 normal) {
        normal = normalize(normal);
        return cross(normal, cross(toProject, normal));
    }

public:
    Camera(uint screenWidth, uint screenHeight): screenWidth(screenWidth), screenHeight(screenHeight) {}
    
    uint getScreenWidth() const {
        return this->screenWidth;
    }
    void setScreenWidth(uint newWidth) {
        this->screenWidth = newWidth;
    }

    uint getScreenHeight() const {
        return this->screenHeight;
    }
    void setScreenHeight(uint newHeight) {
        this->screenHeight = newHeight;
    }

    float getFov() const {
        return this->fov;
    }
    void setFov(float newFov) {
        this->fov = newFov;
    }

    float getZNear() const {
        return this->zNear;
    }
    void setZNear(float newZNear) {
        this->zNear = newZNear;
    }

    float getZFar() const {
        return this->zFar;
    }
    void setZFar(float newZFar) {
        this->zFar = newZFar;
    }

    vec3 getPosition() const {
        return this->position;
    }
    void setPosition(vec3 newPosition) {
        this->position = newPosition;
    }

    vec3 getEulerAngle() const {
        return this->eulerAngle;
    }
    void setEulerAngle(vec3 newEulerAngle) {
        this->eulerAngle = newEulerAngle;
    }

    vec3 getTarget() const {
        return this->target;
    }
    void setTarget(vec3 newTarget) {
        this->target = newTarget;
    }

    float getTargetDistance() const {
        return this->targetDistance;
    }
    void setTargetDistance(float newTargetDistance) {
        this->targetDistance = newTargetDistance;
    }

    float getTranslationSpeed() const {
        return this->translationSpeed;
    }
    void setTranslationSpeed(float newTranslationSpeed) {
        this->translationSpeed = newTranslationSpeed;
    }

    float getRotationSpeed() const {
        return this->rotationSpeed;
    }
    void setRotationSpeed(float newRotationSpeed) {
        this->rotationSpeed = newRotationSpeed;
    }

    void update(float deltaTime) {
        this->eulerAngle = vec3(clipAngle180(this->eulerAngle.x), clipAngle180(this->eulerAngle.y), clipAngle180(this->eulerAngle.z));
        this->eulerAngle.x = clamp(this->eulerAngle.x, -89.f, 89.f);
        this->rotation = quat(this->eulerAngle * M_PI / 180.0f);
        
        if (this->targetDistance < this->zNear) {
            this->targetDistance = this->zNear;
        }
        
        if (this->targetDistance > this->zFar) {
            this->targetDistance = this->zFar;
        }

        if (this->mode == LOOK_AT) {
	        this->position = this->target - (this->rotation * VEC_FRONT) * this->targetDistance;
        }

        this->projectionMatrix = perspective(radians(this->fov), ((float) this->screenWidth) / this->screenHeight, this->zNear, this->zFar);

        const vec3 front = this->rotation * VEC_FRONT;
        const vec3 up = this->rotation * VEC_UP;

        this->viewMatrix = lookAt(this->position, this->position + front, VEC_UP);
    }

    void update(float deltaTime, vec3 newTarget) {
        this->target = newTarget;
        this->update(deltaTime);
    }

    mat4 getView() const {
        return this->viewMatrix;
    }

    mat4 getProjection() const {
        return this->projectionMatrix;
    }

    void rotateWithMouse(float deltaX, float deltaY) {
        const static float ROTATE_MOUSE_FACTOR = 500.f;

        if (deltaY != 0) {
            this->eulerAngle.x += (deltaY * this->rotationSpeed) / ROTATE_MOUSE_FACTOR;
        }

        if (deltaX != 0) {
            this->eulerAngle.y += (-deltaX * this->rotationSpeed) / ROTATE_MOUSE_FACTOR;
        }
    }

    void forward(float deltaTime) {
        vec3 direction = this->rotation * vec3(0.0f, 0.0f, 1.0f);

        switch (this->mode) {
        case LOOK_AT:
            this->targetDistance -= deltaTime * this->translationSpeed;
            break;
        case FRONT:
            this->position += normalize(direction) * this->translationSpeed * deltaTime;
            break;
        case HOVER:
            this->position += normalize(this->projectVectorOnPlan(direction, vec3(0.0f, 1.0f, 0.0f))) * this->translationSpeed * deltaTime;
            break;
        }
    }

    void backward(float deltaTime) {
        vec3 direction = this->rotation * vec3(0.0f, 0.0f, -1.0f);

        switch (this->mode) {
        case LOOK_AT:
            this->targetDistance += deltaTime * this->translationSpeed;
            break;
        case FRONT:
            this->position += normalize(direction) * this->translationSpeed * deltaTime;
            break;
        case HOVER:
            this->position += normalize(this->projectVectorOnPlan(direction, vec3(0.0f, 1.0f, 0.0f))) * this->translationSpeed * deltaTime;
            break;
        }
    }

    void left(float deltaTime) {
        switch (this->mode) {
        case LOOK_AT:
            break;
        case FRONT:
        case HOVER:
            this->position += normalize(this->rotation * vec3(1.0f, 0.0f, 0.0f)) * this->translationSpeed * deltaTime;
            break;
        }        
    }

    void right(float deltaTime) {
        switch (this->mode) {
        case LOOK_AT:
            break;
        case FRONT:
        case HOVER:
            this->position += normalize(this->rotation * vec3(-1.0f, 0.0f, 0.0f)) * this->translationSpeed * deltaTime;
            break;
        }        
    }

    void changeMode() {
        ++this->mode;
        if (this->mode == LOOK_AT) {
            this->targetDistance = distance(this->position, this->target);
        }
    }
};

#endif //CAMERA