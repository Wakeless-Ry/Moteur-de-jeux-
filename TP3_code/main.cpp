#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <unordered_map>
#include "src/Mesh.hpp"
#include "src/Camera.hpp"
#include "src/Controls.hpp"
#include "src/GameEngine.hpp"
#include "src/Scenegraph.hpp"
#include "src/Scenenode.hpp"
#include "src/Transform.hpp"

GLFWwindow* window;
int fps_scaling = 1;

class MoteurSceneGraph : public GameEngine {
    SceneGraph sceneGraph;
    float animationTime;

    Mesh* sphereMesh;

    void init() override {
        animationTime = 0.0f;

        glfwPollEvents();
        glfwSetCursorPos(this->getWindow(),
            this->getCamera().getScreenWidth() / 2,
            this->getCamera().getScreenHeight() / 2);

        Controls& controls = this->getControls();

        // mouse
        controls.addMouseDeltaCallback(new MouseMoveCallback([this](float dx, float dy) {
            this->getCamera().rotateWithMouse(dx, dy);
        }));

        // quit
        controls.addKeyPressedCallback(GLFW_KEY_ESCAPE,
            new KeyCallback([this](float) { this->stopRunning(); }));

        // move cam
        controls.addKeyDownCallback(GLFW_KEY_W,
            new KeyCallback([this](float dt) { this->getCamera().forward(dt); }));

        controls.addKeyDownCallback(GLFW_KEY_S,
            new KeyCallback([this](float dt) { this->getCamera().backward(dt); }));

        controls.addKeyDownCallback(GLFW_KEY_A,
            new KeyCallback([this](float dt) { this->getCamera().left(dt); }));

        controls.addKeyDownCallback(GLFW_KEY_D,
            new KeyCallback([this](float dt) { this->getCamera().right(dt); }));

        controls.addKeyPressedCallback(GLFW_KEY_1,
            new KeyCallback([this](float)
            { 
                fps_scaling++;
                if (fps_scaling == 3)  
                    fps_scaling = 4;
                if (fps_scaling > 4) 
                    fps_scaling = 1;
                this->setTargetFPS(fps_scaling * 30.0f);
            }));
        setupScene();
    }

    struct BodyNodes
    {
        SceneNode* orbit;
        SceneNode* spin;
    };

    std::unordered_map<std::string, BodyNodes> bodies;

    SceneNode* createPlanet(const std::string& name, SceneNode* parent, float orbitDistance,
        float scale, float tiltAngle, const std::string& texturePath)
    {
        vector<Triangle> dummy = {
            Triangle(glm::vec3(0,0,0), glm::vec3(1,0,0), glm::vec3(0,1,0))
        };

        // Orbite
        SceneNode* orbit = sceneGraph.createNode(name + "_Orbit", parent);

        // tilt
        SceneNode* tilt = sceneGraph.createNode(name + "_Tilt", orbit);
        tilt->getTransform().setLocalRotationEuler(0.0f, 0.0f, tiltAngle);

        // Spin
        SceneNode* spin = sceneGraph.createNode(name + "_Spin", tilt);
        spin->getTransform().setLocalPosition(orbitDistance, 0.0f, 0.0f);

        // Mesh
        Mesh* mesh = new Mesh( "shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl", dummy);

        mesh->loadOFF("assets/off/sphere.off");
        mesh->addTexture(texturePath.c_str(), "sphereTexture");

        SceneNode* body = sceneGraph.createNode(name, spin);
        body->getTransform().setLocalScale(scale, scale, scale);
        body->setMesh(mesh);
        bodies[name] = { orbit, spin };
        return spin;
    }

    SceneNode* createMoon(const std::string& name,SceneNode* parentSpin, float orbitDistance,
        float scale, float tiltAngle, const std::string& texturePath)
    {
        vector<Triangle> dummy = {
            Triangle(glm::vec3(0,0,0), glm::vec3(1,0,0), glm::vec3(0,1,0))
        };

        SceneNode* orbit = sceneGraph.createNode(name + "_Orbit", parentSpin);

        SceneNode* tilt = sceneGraph.createNode(name + "_Tilt", orbit);
        tilt->getTransform().setLocalRotationEuler(0.0f, 0.0f, tiltAngle);

        SceneNode* spin = sceneGraph.createNode(name + "_Spin", tilt);
        spin->getTransform().setLocalPosition(orbitDistance, 0.0f, 0.0f);

        Mesh* mesh = new Mesh(
            "shaders/vertex_shader.glsl",
            "shaders/fragment_shader.glsl",
            dummy
        );

        mesh->loadOFF("assets/off/sphere.off");
        mesh->addTexture(texturePath.c_str(), "sphereTexture");

        SceneNode* body = sceneGraph.createNode(name, spin);
        body->getTransform().setLocalScale(scale, scale, scale);
        body->setMesh(mesh);
        bodies[name] = { orbit, spin };
        return spin;
    }

    void setupScene()
    {
        // Soleil
        SceneNode* SunRoot = sceneGraph.createNode("SunRoot");
        SunRoot->getTransform().setLocalPosition(0.0f, 0.0f, 0.0f);
        float scale = 1.5f;
        SceneNode* sunSpin = createPlanet("Sun", SunRoot, 0.0f, scale * 2.5f, 7.25f, "assets/textures/sun.jpg");
        
        // Mercure
        createPlanet("Mercury", SunRoot, scale * 4.0f, scale * 0.2f, 0.03f, "assets/textures/mercury.jpg");

        // Vénus
        createPlanet("Venus", SunRoot, scale * 5.5f, scale * 0.475f, 177.0f, "assets/textures/venus.jpg");

        // Terre
        SceneNode* earthSpin = createPlanet("Earth", SunRoot, scale * 7.5f, scale * 0.5f, 23.44f, "assets/textures/earth.jpg");

        // Lune
        createMoon("Moon", earthSpin, scale * 1.0f, scale * 0.135f, 6.68f, "assets/textures/moon.png");

        // Mars
        createPlanet("Mars", SunRoot, scale * 10.0f, scale * 0.265f, 25.0f, "assets/textures/mars.jpg");
        SceneNode* marsSpin = bodies["Mars"].spin;
        createMoon("Phobos", marsSpin, 0.5f, 0.05f, 0.0f, "assets/textures/moon.png");
        createMoon("Deimos", marsSpin, 0.8f, 0.04f, 0.0f, "assets/textures/moon.png");

        // Jupiter
        createPlanet("Jupiter", SunRoot, scale * 15.0f, scale * 1.25f, 3.1f, "assets/textures/jupiter.png");
        SceneNode* jupiterSpin = bodies["Jupiter"].spin;
        createMoon("Io",        jupiterSpin, 1.2f, 0.15f, 0.0f, "assets/textures/moon.png");
        createMoon("Europa",    jupiterSpin, 1.6f, 0.14f, 0.0f, "assets/textures/moon.png");
        createMoon("Ganymede",  jupiterSpin, 2.0f, 0.2f,  0.0f, "assets/textures/moon.png");
        createMoon("Callisto",  jupiterSpin, 2.5f, 0.18f, 0.0f, "assets/textures/moon.png");

        // Saturne
        createPlanet("Saturn", SunRoot, scale * 19.0f, scale * 1.05f, 26.7f, "assets/textures/saturn.png");
        SceneNode* saturnSpin = bodies["Saturn"].spin;
        createMoon("Titan", saturnSpin, 1.5f, 0.18f, 0.0f, "assets/textures/moon.png");

        // Uranus
        createPlanet("Uranus", SunRoot, scale * 23.0f, scale * 0.8f, 97.8f, "assets/textures/uranus.png");
        SceneNode* uranusSpin = bodies["Uranus"].spin;
        createMoon("Titania", uranusSpin, 1.2f, 0.15f, 0.0f, "assets/textures/moon.png");

        // Neptune
        createPlanet("Neptune", SunRoot, scale * 27.0f, scale * 0.775f, 28.3f, "assets/textures/neptune.png");
        SceneNode* neptuneSpin = bodies["Neptune"].spin;
        createMoon("Triton", neptuneSpin, 1.3f, 0.16f, 157.0f, "assets/textures/moon.png");
    }

    void processInput(float) override {}

    void update(float delta) override
    {
        float timeScale = 10.0f;
        animationTime += delta * timeScale;

        auto animateBody = [&](const std::string& name, float orbitSpeed, float spinSpeed)
        {
            auto it = bodies.find(name);
            if (it == bodies.end()) return;

            SceneNode* orbit = it->second.orbit;
            SceneNode* spin  = it->second.spin;

            orbit->getTransform().setLocalRotationEuler(
                0.0f,
                animationTime * orbitSpeed,
                0.0f
            );

            spin->getTransform().setLocalRotationEuler(
                0.0f,
                animationTime * spinSpeed,
                0.0f
            );
        };

        // solid
        animateBody("Mercury", 4.7f, 20.0f);
        animateBody("Venus",   3.5f,  5.0f);
        animateBody("Earth",   3.0f, 25.0f);
        animateBody("Mars",    2.4f, 22.0f);

        // gaz
        animateBody("Jupiter", 1.3f, 40.0f);
        animateBody("Saturn",  1.0f, 35.0f);
        animateBody("Uranus",  0.7f, 30.0f);
        animateBody("Neptune", 0.5f, 28.0f);

        // moon
        animateBody("Moon", 8.0f, 10.0f);
        animateBody("Moon", 8.0f, 10.0f);
        animateBody("Phobos", 20.0f, 5.0f);
        animateBody("Deimos", 15.0f, 4.0f);
        animateBody("Io", 15.0f, 8.0f);
        animateBody("Europa", 12.0f, 7.0f);
        animateBody("Ganymede", 10.0f, 6.0f);
        animateBody("Callisto", 8.0f, 5.0f);
        animateBody("Titan", 6.0f, 4.0f);
        animateBody("Titania", 5.0f, 3.0f);
        animateBody("Triton", 4.0f, 3.0f);
        sceneGraph.update(delta);
    }

    void render(float) override {
        sceneGraph.render(this->getCamera());
    }

    void cleanUp() override {
        sceneGraph.clear();
    }

public:
    MoteurSceneGraph(GLFWwindow* window,
                     uint width,
                     uint height)
        : GameEngine(window, width, height) {}
};

int main() {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
        1280, 720,
        "Scene Graph Engine",
        NULL, NULL
    );

    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glLineWidth(1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    MoteurSceneGraph engine(window, width, height);
    engine.run();

    glfwTerminate();
    return 0;
}