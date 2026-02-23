#ifndef TRANSFORM
#define TRANSFORM

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {
private:
    glm::vec3 localPosition;
    glm::quat localRotation;
    glm::vec3 localScale;
    
    glm::vec3 worldPosition;
    glm::quat worldRotation;
    glm::vec3 worldScale;
    
    glm::mat4 localMatrix;
    glm::mat4 worldMatrix;
    
    bool isDirty;

public:
    Transform() 
        : localPosition(0.0f, 0.0f, 0.0f),
          localRotation(1.0f, 0.0f, 0.0f, 0.0f),
          localScale(1.0f, 1.0f, 1.0f),
          worldPosition(0.0f, 0.0f, 0.0f),
          worldRotation(1.0f, 0.0f, 0.0f, 0.0f),
          worldScale(1.0f, 1.0f, 1.0f),
          isDirty(true) {
        updateLocalMatrix();
    }

    // Position
    void setLocalPosition(const glm::vec3& position) {
        localPosition = position;
        isDirty = true;
    }

    void setLocalPosition(float x, float y, float z) {
        setLocalPosition(glm::vec3(x, y, z));
    }

    glm::vec3 getLocalPosition() const {
        return localPosition;
    }

    glm::vec3 getWorldPosition() const {
        return worldPosition;
    }

    // Rotation
    void setLocalRotation(const glm::quat& rotation) {
        localRotation = rotation;
        isDirty = true;
    }

    void setLocalRotation(float angle, const glm::vec3& axis) {
        localRotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
        isDirty = true;
    }

    void setLocalRotationEuler(float pitch, float yaw, float roll) {
        localRotation = glm::quat(glm::vec3(
            glm::radians(pitch),
            glm::radians(yaw),
            glm::radians(roll)
        ));
        isDirty = true;
    }

    glm::quat getLocalRotation() const {
        return localRotation;
    }

    glm::quat getWorldRotation() const {
        return worldRotation;
    }

    glm::vec3 getLocalRotationEuler() const {
        return glm::degrees(glm::eulerAngles(localRotation));
    }

    // Scale
    void setLocalScale(const glm::vec3& scale) {
        localScale = scale;
        isDirty = true;
    }

    void setLocalScale(float x, float y, float z) {
        setLocalScale(glm::vec3(x, y, z));
    }

    void setLocalScale(float uniform) {
        setLocalScale(glm::vec3(uniform, uniform, uniform));
    }

    glm::vec3 getLocalScale() const {
        return localScale;
    }

    glm::vec3 getWorldScale() const {
        return worldScale;
    }

    // Operations
    void translate(const glm::vec3& translation) {
        localPosition += translation;
        isDirty = true;
    }

    void rotate(float angle, const glm::vec3& axis) {
        localRotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis)) * localRotation;
        isDirty = true;
    }

    void rotateEuler(float pitch, float yaw, float roll) {
        glm::quat rotation = glm::quat(glm::vec3(
            glm::radians(pitch),
            glm::radians(yaw),
            glm::radians(roll)
        ));
        localRotation = rotation * localRotation;
        isDirty = true;
    }

    void scale(const glm::vec3& scaleFactor) {
        localScale *= scaleFactor;
        isDirty = true;
    }

    glm::vec3 getForward() const {
        return glm::normalize(worldRotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 getRight() const {
        return glm::normalize(worldRotation * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 getUp() const {
        return glm::normalize(worldRotation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void updateLocalMatrix() {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), localPosition);
        glm::mat4 rotationMatrix = glm::toMat4(localRotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), localScale);
        
        localMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        isDirty = true;
    }

    void updateWorldMatrix(const glm::mat4& parentWorldMatrix) {
        updateLocalMatrix();
        worldMatrix = parentWorldMatrix * localMatrix;
        
        // world position
        worldPosition = glm::vec3(worldMatrix[3]);
        
        //scale
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(worldMatrix[0]));
        scale.y = glm::length(glm::vec3(worldMatrix[1]));
        scale.z = glm::length(glm::vec3(worldMatrix[2]));
        worldScale = scale;
        
        // rotation
        glm::mat3 rotationMatrix;
        rotationMatrix[0] = glm::vec3(worldMatrix[0]) / scale.x;
        rotationMatrix[1] = glm::vec3(worldMatrix[1]) / scale.y;
        rotationMatrix[2] = glm::vec3(worldMatrix[2]) / scale.z;
        worldRotation = glm::quat_cast(rotationMatrix);
        
        isDirty = false;
    }

    glm::mat4 getLocalMatrix() const {
        return localMatrix;
    }

    glm::mat4 getWorldMatrix() const {
        return worldMatrix;
    }

    bool dirty() const {
        return isDirty;
    }

    void markDirty() {
        isDirty = true;
    }

    void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)) {
        glm::vec3 direction = glm::normalize(target - localPosition);
        glm::vec3 right = glm::normalize(glm::cross(up, direction));
        glm::vec3 newUp = glm::cross(direction, right);
        
        glm::mat3 rotationMatrix;
        rotationMatrix[0] = right;
        rotationMatrix[1] = newUp;
        rotationMatrix[2] = direction;
        
        localRotation = glm::quat_cast(rotationMatrix);
        isDirty = true;
    }
};

#endif // TRANSFORM