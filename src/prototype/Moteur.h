#include "src/GameEngine.h"

class Moteur : public GameEngine {

    void init() override {
        this->initInputs();
        this->initScene();
    }

    void initScene() {}

    void initInputs() {}

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {}

    void render(float deltaTime) override {}

    void cleanUp() override {}

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {}
};