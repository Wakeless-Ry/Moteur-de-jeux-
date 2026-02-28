#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Controls.h"
#include "Scene.h"

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

    void initInternal();
    void processInputInternal();
    void updateInternal();
    void renderInternal();
    void cleanUpInternal();

  public:
    GameEngine(GLFWwindow *window, uint width, uint height);

    int run();

    void stopRunning();

    GLFWwindow *getWindow();
    Camera &getCamera();
    Controls &getControls();
    Scene &getScene();
    void setScene(const Scene &other);

    virtual void init() {};
    virtual void processInput(float deltaTime) {};
    virtual void update(float deltaTime) {};
    virtual void render(float deltaTime) {};
    virtual void cleanUp() {};
};

#endif // GAME_ENGINE