#ifndef CONTROLS
#define CONTROLS

#include <map>
#include <functional>

using namespace std;

using uint = unsigned int;

class Controls {
    map<uint, vector<function<void(float)>>> keyPressedMapCallback;
    map<uint, vector<function<void(float)>>> keyReleasedMapCallback;
    map<uint, vector<function<void(float)>>> keyDownMapCallback;

    map<uint, bool> keysToWatch;

    vector<function<void(float, float)>> mouseDeltaCallback;

    void processMouse(GLFWwindow *window, Camera &camera) {
        double currentX, currentY;
        glfwGetCursorPos(window, &currentX, &currentY);

        float centerX = camera.getScreenWidth() / 2.;
        float centerY = camera.getScreenHeight() / 2.;

        float deltaX = currentX - centerX;
        float deltaY = currentY - centerY;
        static int cpt = 0;

        if (cpt < 5) {
            cpt++;
        } else {
            this->handleMouseDelta(deltaX, deltaY);
        }

        glfwSetCursorPos(window, centerX, centerY);
    }

    void callEach(vector<function<void(float)>> callbacks, float value) {
        for (function<void(float)> callback : callbacks) {
            callback(value);
        }
    }

    void processKeys(GLFWwindow *window, float deltaTime) {
        for (auto &pair : this->keysToWatch) {
            uint key = pair.first;
            bool wasPressed = pair.second;

            if (glfwGetKey(window, key) == GLFW_PRESS) {
                if (wasPressed) {
                    this->callEach(this->keyDownMapCallback[key], deltaTime);
                } else {
                    this->callEach(this->keyPressedMapCallback[key], deltaTime);
                    this->callEach(this->keyDownMapCallback[key], deltaTime);
                    this->keysToWatch[key] = true;
                }
            } else {
                if (wasPressed) {
                    this->callEach(this->keyReleasedMapCallback[key], deltaTime);
                    this->keysToWatch[key] = false;
                }
            }
        }
    }

public:
    void addKeyPressedCallback(uint key, function<void(float)> callback) {
        this->keysToWatch[key] = false;
        this->keyPressedMapCallback[key].push_back(callback);
    }
    
    void handleKeyPressed(uint key, float deltaTime) {
        auto it = this->keyPressedMapCallback.find(key);
        if (it != this->keyPressedMapCallback.end()) {
            for (const auto& callback : it->second) {
                callback(deltaTime);
            }
        }
    }
    void addKeyReleasedCallback(uint key, function<void(float)> callback) {
        this->keysToWatch[key] = false;
        this->keyReleasedMapCallback[key].push_back(callback);
    }
    
    void handleKeyReleased(uint key, float deltaTime) {
        auto it = this->keyReleasedMapCallback.find(key);
        if (it != this->keyReleasedMapCallback.end()) {
            for (const auto& callback : it->second) {
                callback(deltaTime);
            }
        }
    }
    void addKeyDownCallback(uint key, function<void(float)> callback) {
        this->keysToWatch[key] = false;
        this->keyDownMapCallback[key].push_back(callback);
    }
    
    void handleKeyDown(uint key, float deltaTime) {
        auto it = this->keyDownMapCallback.find(key);
        if (it != this->keyDownMapCallback.end()) {
            for (const auto& callback : it->second) {
                callback(deltaTime);
            }
        }
    }

    void addMouseDeltaCallback(function<void(float, float)> callback) {
        this->mouseDeltaCallback.push_back(callback);
    }

    void handleMouseDelta(float deltaX, float deltaY) {
        for (function<void(float, float)> callback : this->mouseDeltaCallback) {
            callback(deltaX, deltaY);
        }
    }

    void processInput(GLFWwindow *window, Camera &camera, float deltaTime) {
        this->processKeys(window, deltaTime);
        this->processMouse(window, camera);
    }
};

#endif //CONTROLS