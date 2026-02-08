#ifndef CONTROLS
#define CONTROLS

#include <map>
#include <functional>

using namespace std;

using uint = unsigned int;

using KeyId = uint;

class KeyCallback {
public:
    function<void(float)> callback;

    KeyCallback(function<void(float)> callback): callback(callback) {}
};

class MouseMoveCallback {
public:
    function<void(float, float)> callback;

    MouseMoveCallback(function<void(float, float)> callback): callback(callback) {}
};

class Controls {
    map<KeyId, vector<KeyCallback *>> keyPressedMapCallback;
    map<KeyId, vector<KeyCallback *>> keyReleasedMapCallback;
    map<KeyId, vector<KeyCallback *>> keyDownMapCallback;

    map<KeyId, bool> keysToWatch;

    vector<MouseMoveCallback *> mouseDeltaCallback;

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
            for (MouseMoveCallback *mouseMoveCallback : this->mouseDeltaCallback) {
                mouseMoveCallback->callback(deltaX, deltaY);
            }
        }

        glfwSetCursorPos(window, centerX, centerY);
    }

    void callEach(vector<KeyCallback *> callbacks, float value) {
        for (KeyCallback *keyCallback : callbacks) {
            keyCallback->callback(value);
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
    void addKeyPressedCallback(KeyId key, KeyCallback *callback) {
        this->keysToWatch[key] = false;
        this->keyPressedMapCallback[key].push_back(callback);
    }

    void addKeyReleasedCallback(KeyId key, KeyCallback *callback) {
        this->keysToWatch[key] = false;
        this->keyReleasedMapCallback[key].push_back(callback);
    }

    void addKeyDownCallback(KeyId key, KeyCallback *callback) {
        this->keysToWatch[key] = false;
        this->keyDownMapCallback[key].push_back(callback);
    }

    void addMouseDeltaCallback(MouseMoveCallback *callback) {
        this->mouseDeltaCallback.push_back(callback);
    }

    void processInput(GLFWwindow *window, Camera &camera, float deltaTime) {
        this->processKeys(window, deltaTime);
        this->processMouse(window, camera);
    }
};

#endif //CONTROLS