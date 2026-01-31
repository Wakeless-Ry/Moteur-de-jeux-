#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>

using uint = unsigned int;

enum CameraMode {
    LOOK_AT,
    DIRECTION,
};

class Camera {
    uint screenWidth;
    uint screenHeight;

    float fov = 45;
    float zNear = .1;
    float zFar = 100;

    glm::vec3 position = glm::vec3(0.0f, 3.0f, 3.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float speed = 2.5;
    float zoom = 1;

    float mode = LOOK_AT;

public:
    Camera(uint screenWidth, uint screenHeight): screenWidth(screenWidth), screenHeight(screenHeight) {}

    uint getScreenWidth() const {
        return this->screenWidth;
    }
    
    void setScreenWidth(uint width) {
        this->screenWidth = width;
    }
    
    uint getScreenHeight() const {
        return this->screenHeight;
    }
    
    void setScreenHeight(uint height) {
        this->screenHeight = height;
    }
    
    float getFov() const {
        return this->fov;
    }
    
    void setFov(float fieldOfView) {
        this->fov = fieldOfView;
    }
    
    float getZNear() const {
        return this->zNear;
    }
    
    void setZNear(float near) {
        this->zNear = near;
    }
    
    float getZFar() const {
        return this->zFar;
    }
    
    void setZFar(float far) {
        this->zFar = far;
    }
    
    glm::vec3 getPosition() const {
        return this->position;
    }
    
    void setPosition(const glm::vec3& pos) {
        this->position = pos;
    }
    
    glm::vec3 getTarget() const {
        return this->target;
    }
    
    void setTarget(const glm::vec3& target) {
        this->target = target;
    }
    
    glm::vec3 getUp() const {
        return this->up;
    }
    
    void setUp(const glm::vec3& upVector) {
        this->up = upVector;
    }

    float getSpeed() const {
        return this->speed;
    }

    void setSpeed(float speed) {
        this->speed = speed;
    }

    float getZoom() const {
        return this->zoom;
    }

    void setZoom(float zoom) {
        this->zoom = zoom;
    }

    float getMode() const {
        return this->mode;
    }

    void setMode(float mode) {
        this->mode = mode;
    }

    glm::mat4 getView() {
        return glm::lookAt(this->position, this->target, this->up);
    }

    glm::mat4 getProjection() {
        return glm::perspective(glm::radians(this->fov), ((float) this->screenWidth) / this->screenHeight, this->zNear, this->zFar);
    }

    void move(float delta) {
        this->position += this->target * delta * this->speed;
    }
};

#endif //CAMERA