#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Camera.hpp"
#include "Controls.hpp"
#include "Scene.hpp"

using namespace std;
using namespace glm;
using ushort = unsigned short;
using uint = unsigned int;

class GameEngine {
    GLFWwindow *window;

    float deltaTime = 0;
    float lastFrame = 0;

    Camera camera;
    Controls controls;
    Scene scene;

    GLuint vertexArrayId;

    bool stop = false;

    void initInternal() {
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        this->init();
    }

    void processInputInternal() {
        controls.processInput(this->window, this->camera, this->deltaTime);

        this->processInput(this->deltaTime);
    }

    void updateInternal() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera.setScreenWidth(width);
        camera.setScreenHeight(height);

        this->update(this->deltaTime);
    }

    void renderInternal() {
        this->camera.update(this->deltaTime);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->scene.draw(this->camera);

        this->render(this->deltaTime);
    }

    void cleanUpInternal() {
        this->scene.cleanUp();

        glDeleteVertexArrays(1, &this->vertexArrayId);

        glfwSetWindowShouldClose(window, true);

        glfwTerminate();

        this->cleanUp();
    }

  public:
    GameEngine(GLFWwindow *window, uint width, uint height)
        : camera(width, height), controls() {
        this->window = window;
    }

    int run() {
        this->initInternal();

        while (!this->stop) {
            float currentFrame = glfwGetTime();
            this->deltaTime = currentFrame - this->lastFrame;
            this->lastFrame = currentFrame;

            this->processInputInternal();
            this->updateInternal();
            this->renderInternal();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        this->cleanUpInternal();

        return 0;
    }

    void stopRunning() { this->stop = true; }

    GLFWwindow *getWindow() { return this->window; }

    Camera &getCamera() { return this->camera; }

    Controls &getControls() { return this->controls; }

    Scene &getScene() { return this->scene; }
    void setScene(const Scene &other) { this->scene = other; }

    virtual void init() {};
    virtual void processInput(float deltaTime) {};
    virtual void update(float deltaTime) {};
    virtual void render(float deltaTime) {};
    virtual void cleanUp() {};
};

#endif // GAME_ENGINE