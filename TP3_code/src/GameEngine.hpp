#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>

#include "src/Mesh.hpp"
#include "src/Camera.hpp"
#include "src/Controls.hpp"

using namespace std;
using namespace glm;
using ushort = unsigned short;
using uint = unsigned int;

class GameEngine {
    GLFWwindow *window;
    float targetFPS = 30.0f;
    float targetFrameTime = 1.0f / targetFPS;
    float deltaTime = 0;
    float lastFrame = 0;
    
    Camera camera;
    Controls controls;
    map<uint, Mesh *> meshes;
    
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

        this->render(this->deltaTime);
    }

    void cleanUpInternal() {
        for (auto &pair : this->meshes) {
            pair.second->cleanUp();
        }

        glDeleteVertexArrays(1, &this->vertexArrayId);
        
        glfwSetWindowShouldClose(window, true);
        
        glfwTerminate();

        this->cleanUp();
    }

public:
    GameEngine(GLFWwindow *window, uint width, uint height): camera(width, height), controls() {
        this->window = window;
    }

    int run()
    {
        this->initInternal();

        while (!this->stop)
        {
            float frameStart = glfwGetTime();

            float currentFrame = frameStart;
            this->deltaTime = currentFrame - this->lastFrame;
            this->lastFrame = currentFrame;

            this->processInputInternal();
            this->updateInternal();
            this->renderInternal();

            glfwSwapBuffers(window);
            glfwPollEvents();

            float frameEnd = glfwGetTime();
            float frameDuration = frameEnd - frameStart;

            if (frameDuration < targetFrameTime)
            {
                float sleepTime = targetFrameTime - frameDuration;
                std::this_thread::sleep_for(
                    std::chrono::duration<float>(sleepTime)
                );
            }

            // fps
            static float timer = 0.0f;
            static int frameCount = 0;

            timer += glfwGetTime() - frameStart;
            frameCount++;

            if (timer >= 1.0f)
            {
                float fps = frameCount / timer;

                std::string title =
                    "Scene Graph Engine | FPS: " + std::to_string((int)fps) +
                    " | Limit: " + std::to_string((int)targetFPS);

                glfwSetWindowTitle(window, title.c_str());

                frameCount = 0;
                timer = 0.0f;
            }
        }

        this->cleanUpInternal();
        return 0;
    }

    void setTargetFPS(float fps)
    {
    targetFPS = fps;
    targetFrameTime = 1.0f / targetFPS;
    }
    void stopRunning() {
        this->stop = true;
    }

    GLFWwindow * getWindow() {
        return this->window;
    }

    Camera & getCamera() {
        return this->camera;
    }

    Controls & getControls() {
        return this->controls;
    }

    uint addMesh(Mesh *mesh) {
        static uint meshIdCpt = 0;
        uint id = meshIdCpt++;
        this->meshes[id] = mesh;
        return id;
    }

    void replaceMesh(uint id, Mesh *mesh) {
        this->meshes[id] = mesh;
    }

    Mesh * getMesh(uint id) {
        return this->meshes[id];
    }

    void removeMesh(uint id) {
        this->meshes.erase(id);
    }

    virtual void init() {};
    virtual void processInput(float delta) {};
    virtual void update(float delta) {};
    virtual void render(float delta) {};
    virtual void cleanUp() {};
};

#endif //GAME_ENGINE