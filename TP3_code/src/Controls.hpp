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

class MouseKeyCallback {
public:
    function<void(float, float, float)> callback;

    MouseKeyCallback(function<void(float, float, float)> callback): callback(callback) {}
};

class Controls {
    map<KeyId, vector<KeyCallback *>> keyPressedMapCallback;
    map<KeyId, vector<KeyCallback *>> keyReleasedMapCallback;
    map<KeyId, vector<KeyCallback *>> keyDownMapCallback;

    map<KeyId, bool> keysToWatch;

    vector<MouseMoveCallback *> mouseDeltaCallback;
    map<KeyId, vector<MouseKeyCallback *>> mouseKeyPressedMapCallback;
    map<KeyId, vector<MouseKeyCallback *>> mouseKeyReleasedMapCallback;
    map<KeyId, vector<MouseKeyCallback *>> mouseKeyDownMapCallback;

    map<KeyId, bool> mouseKeysToWatch;

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

    void callEachKey(vector<KeyCallback *> callbacks, float value) {
        for (KeyCallback *keyCallback : callbacks) {
            keyCallback->callback(value);
        }
    }

    void callEachMouseKey(vector<MouseKeyCallback *> callbacks, float posX, float posY, float value) {
        for (MouseKeyCallback *keyCallback : callbacks) {
            keyCallback->callback(posX, posY, value);
        }
    }

    void processKeys(GLFWwindow *window, float deltaTime) {
        for (auto &pair : this->keysToWatch) {
            uint key = pair.first;
            bool wasPressed = pair.second;

            if (glfwGetKey(window, key) == GLFW_PRESS) {
                if (wasPressed) {
                    this->callEachKey(this->keyDownMapCallback[key], deltaTime);
                } else {
                    this->callEachKey(this->keyPressedMapCallback[key], deltaTime);
                    this->callEachKey(this->keyDownMapCallback[key], deltaTime);
                    this->keysToWatch[key] = true;
                }
            } else {
                if (wasPressed) {
                    this->callEachKey(this->keyReleasedMapCallback[key], deltaTime);
                    this->keysToWatch[key] = false;
                }
            }
        }
    }

    void processMouseKeys(GLFWwindow *window, Camera &camera, float deltaTime) {
        double currentX, currentY;
        glfwGetCursorPos(window, &currentX, &currentY);

        for (auto &pair : this->mouseKeysToWatch) {
            uint key = pair.first;
            bool wasPressed = pair.second;

            if (glfwGetMouseButton(window, key) == GLFW_PRESS) {
                if (wasPressed) {
                    this->callEachMouseKey(this->mouseKeyDownMapCallback[key], currentX, currentY, deltaTime);
                } else {
                    this->callEachMouseKey(this->mouseKeyPressedMapCallback[key], currentX, currentY, deltaTime);
                    this->callEachMouseKey(this->mouseKeyDownMapCallback[key], currentX, currentY, deltaTime);
                    this->mouseKeysToWatch[key] = true;
                }
            } else {
                if (wasPressed) {
                    this->callEachMouseKey(this->mouseKeyReleasedMapCallback[key], currentX, currentY, deltaTime);
                    this->mouseKeysToWatch[key] = false;
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

    void addMouseKeyPressedCallback(KeyId key, MouseKeyCallback *callback) {
        this->mouseKeysToWatch[key] = false;
        this->mouseKeyPressedMapCallback[key].push_back(callback);
    }

    void addMouseKeyReleasedCallback(KeyId key, MouseKeyCallback *callback) {
        this->mouseKeysToWatch[key] = false;
        this->mouseKeyReleasedMapCallback[key].push_back(callback);
    }

    void addMouseKeyDownCallback(KeyId key, MouseKeyCallback *callback) {
        this->mouseKeysToWatch[key] = false;
        this->mouseKeyDownMapCallback[key].push_back(callback);
    }

    void processInput(GLFWwindow *window, Camera &camera, float deltaTime) {
        this->processKeys(window, deltaTime);
        this->processMouse(window, camera);
        this->processMouseKeys(window, camera, deltaTime);
    }
};

#endif //CONTROLS