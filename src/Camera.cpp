#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

static float clipAngle180(float _angle) {
    while (_angle >= 180.f || _angle < -180.f) {
        _angle += (_angle > 180.f) ? -360.f : 360.f;
    }
    return _angle;
}

Camera::Camera(uint screenWidth, uint screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight) {}

void Camera::update(const vec3 &pos) { this->setTarget(pos); }

uint Camera::getScreenWidth() const { return this->screenWidth; }
void Camera::setScreenWidth(uint newWidth) { this->screenWidth = newWidth; }

uint Camera::getScreenHeight() const { return this->screenHeight; }
void Camera::setScreenHeight(uint newHeight) { this->screenHeight = newHeight; }

float Camera::getFov() const { return this->fov; }
void Camera::setFov(float newFov) { this->fov = newFov; }

float Camera::getZNear() const { return this->zNear; }
void Camera::setZNear(float newZNear) { this->zNear = newZNear; }

float Camera::getZFar() const { return this->zFar; }
void Camera::setZFar(float newZFar) { this->zFar = newZFar; }

vec3 Camera::getPosition() const { return this->position; }
void Camera::setPosition(vec3 newPosition) { this->position = newPosition; }

vec3 Camera::getEulerAngle() const { return this->eulerAngle; }
void Camera::setEulerAngle(vec3 newEulerAngle) {
    this->eulerAngle = newEulerAngle;
}

vec3 Camera::getTarget() const { return this->target; }
void Camera::setTarget(vec3 newTarget) { this->target = newTarget; }

float Camera::getTargetDistance() const { return this->targetDistance; }
void Camera::setTargetDistance(float newTargetDistance) {
    this->targetDistance = newTargetDistance;
}

float Camera::getTranslationSpeed() const { return this->translationSpeed; }
void Camera::setTranslationSpeed(float newTranslationSpeed) {
    this->translationSpeed = newTranslationSpeed;
}

float Camera::getRotationSpeed() const { return this->rotationSpeed; }
void Camera::setRotationSpeed(float newRotationSpeed) {
    this->rotationSpeed = newRotationSpeed;
}

vec3 Camera::projectVectorOnPlan(vec3 toProject, vec3 normal) {
    normal = normalize(normal);
    return cross(normal, cross(toProject, normal));
}

void Camera::update(float deltaTime) {
    if (this->mode == ROTATE) {
        this->eulerAngle.y = this->eulerAngle.y + deltaTime * rotationSpeed;
    }

    this->eulerAngle =
        vec3(clipAngle180(this->eulerAngle.x), clipAngle180(this->eulerAngle.y),
             clipAngle180(this->eulerAngle.z));
    this->eulerAngle.x = clamp(this->eulerAngle.x, -89.f, 89.f);
    this->rotation = quat(this->eulerAngle * (float)M_PI / 180.0f);

    if (this->targetDistance < this->zNear) {
        this->targetDistance = this->zNear;
    }

    if (this->targetDistance > this->zFar) {
        this->targetDistance = this->zFar;
    }

    if (this->mode == LOOK_AT || this->mode == ROTATE) {
        this->position =
            this->target - (this->rotation * VEC_FRONT) * this->targetDistance;
    }

    this->projectionMatrix = perspective(
        radians(this->fov), ((float)this->screenWidth) / this->screenHeight,
        this->zNear, this->zFar);

    const vec3 front = this->rotation * VEC_FRONT;
    const vec3 up = this->rotation * VEC_UP;

    this->viewMatrix = lookAt(this->position, this->position + front, VEC_UP);
}

void Camera::update(float deltaTime, vec3 newTarget) {
    this->target = newTarget;
    this->update(deltaTime);
}

mat4 Camera::getView() const { return this->viewMatrix; }

mat4 Camera::getProjection() const { return this->projectionMatrix; }

void Camera::rotateWithMouse(float deltaX, float deltaY) {
    const static float ROTATE_MOUSE_FACTOR = 500.f;

    if (deltaY != 0) {
        this->eulerAngle.x +=
            (deltaY * this->rotationSpeed) / ROTATE_MOUSE_FACTOR;
    }

    if (deltaX != 0) {
        this->eulerAngle.y +=
            (-deltaX * this->rotationSpeed) / ROTATE_MOUSE_FACTOR;
    }
}

void Camera::forward(float deltaTime) {
    vec3 direction = this->rotation * vec3(0.0f, 0.0f, 1.0f);

    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        this->targetDistance -= deltaTime * this->translationSpeed;
        break;
    case FRONT:
        this->position +=
            normalize(direction) * this->translationSpeed * deltaTime;
        break;
    case HOVER:
        this->position += normalize(this->projectVectorOnPlan(
                              direction, vec3(0.0f, 1.0f, 0.0f))) *
                          this->translationSpeed * deltaTime;
        break;
    case Count:
        break;
    }
}

void Camera::backward(float deltaTime) {
    vec3 direction = this->rotation * vec3(0.0f, 0.0f, -1.0f);

    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        this->targetDistance += deltaTime * this->translationSpeed;
        break;
    case FRONT:
        this->position +=
            normalize(direction) * this->translationSpeed * deltaTime;
        break;
    case HOVER:
        this->position += normalize(this->projectVectorOnPlan(
                              direction, vec3(0.0f, 1.0f, 0.0f))) *
                          this->translationSpeed * deltaTime;
        break;
    case Count:
        break;
    }
}

void Camera::left(float deltaTime) {
    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        break;
    case FRONT:
    case HOVER:
        this->position += normalize(this->rotation * vec3(1.0f, 0.0f, 0.0f)) *
                          this->translationSpeed * deltaTime;
        break;
    case Count:
        break;
    }
}

void Camera::right(float deltaTime) {
    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        break;
    case FRONT:
    case HOVER:
        this->position += normalize(this->rotation * vec3(-1.0f, 0.0f, 0.0f)) *
                          this->translationSpeed * deltaTime;
        break;
    case Count:
        break;
    }
}

void Camera::up_arrow(float deltaTime) {
    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        this->rotationSpeed += this->rotationSpeed * deltaTime;
        break;
    case FRONT:
    case HOVER:
    case Count:
        break;
    }
}

void Camera::down_arrow(float deltaTime) {
    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        this->rotationSpeed -= this->rotationSpeed * deltaTime;
        break;
    case FRONT:
    case HOVER:
    case Count:
        break;
    }
}

void Camera::space(float deltaTime) {
    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        break;
    case FRONT:
    case HOVER:
        this->position += normalize(this->position * vec3(.0f, 1.0f, 0.0f)) *
                          this->translationSpeed * deltaTime;
        break;
    case Count:
        break;
    }
}

void Camera::left_shift(float deltaTime) {
    switch (this->mode) {
    case LOOK_AT:
    case ROTATE:
        break;
    case FRONT:
    case HOVER:
        this->position += normalize(this->position * vec3(.0f, -1.0f, 0.0f)) *
                          this->translationSpeed * deltaTime;
        break;
    case Count:
        break;
    }
}

CameraMode Camera::changeMode() {
    ++this->mode;
    if (this->mode == LOOK_AT || this->mode == ROTATE) {
        this->targetDistance = distance(this->position, this->target);
    }

    return this->mode;
}