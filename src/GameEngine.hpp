#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <iostream>
#include <vector>

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
    
    float deltaTime = 0;
    float lastFrame = 0;
    
    Camera camera;
    Controls controls;
    map<uint, Mesh *> meshes;
    
    GLuint vertexArrayId;

    bool stop = false;
    
    GameEngine(GLFWwindow *window, uint width, uint height): camera(width, height), controls() {
        this->window = window;
    }

    void init() {
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
    }

    void processInput() {
        controls.processInput(this->window, this->camera, this->deltaTime);

    }

    void update() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera.setScreenWidth(width);
        camera.setScreenHeight(height);
    }

    void render() {
        this->camera.update(this->deltaTime);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto &pair : this->meshes) {
            pair.second->draw(this->camera);
        }
    }

    void cleanUp() {
        for (auto &pair : this->meshes) {
            pair.second->cleanUp();
        }

        glDeleteVertexArrays(1, &this->vertexArrayId);
        
        glfwSetWindowShouldClose(window, true);
        
        glfwTerminate();
    }

public:

    static GameEngine newGameEngine(GLFWwindow *window) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        return GameEngine(window, width, height);
    }

    int run() {
        this->init();

        while (!this->stop) {
            float currentFrame = glfwGetTime();
            this->deltaTime = currentFrame - this->lastFrame;
            this->lastFrame = currentFrame;

            this->processInput();
            this->update();
            this->render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        this->cleanUp();

        return 0;
    }

    void stopRunning() {
        this->stop = true;
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
};

#endif //GAME_ENGINE