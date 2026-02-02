// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <glm/ext.hpp>

#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "src/Camera.h"
#include "src/stb_image.h"

using namespace glm;

#include <lib/shader.hpp>
#include <lib/objloader.hpp>
#include <lib/vboindexer.hpp>

#include "src/Mesh.h"

void processInput(GLFWwindow *window, Mesh &terrain);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 camera_position = glm::vec3(0.0f, 3.0f, 3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// rotation
float angle = 0.;
float zoom = 1.;
/*******************************************************************************/
float randomFloat()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 0.4f);
    return dist(gen);
}

int nX = 16;
int nZ = 16;

void generate_scene(std::vector<unsigned int> &indices, std::vector<std::vector<unsigned int>> &triangles, std::vector<glm::vec3> &indexed_vertices, std::vector<glm::vec2> &textures_coords)
{

    float minX = -1.f;
    float minZ = -1.f;

    for (unsigned int i = 0; i < nX; i++)
    {
        for (unsigned int j = 0; j < nZ; j++)
        {

            float x = minX + 2.0 * i / (nX - 1);
            float z = minZ + 2.0 * j / (nZ - 1);
            // float y = 0.f;

            // float y = randomFloat();
            float y = 0.f;
            indexed_vertices.push_back(glm::vec3(x, y, z));
            textures_coords.push_back(glm::vec2((i + 0.5) / nX, (j + 0.5) / nZ));
        }
    }

    for (unsigned int i = 0; i < nX - 1; i++)
    {
        for (unsigned int j = 0; j < nZ - 1; j++)
        {

            triangles.push_back({i * nZ + j,
                                 (i + 1) * nZ + j,
                                 (i + 1) * nZ + (j + 1)});

            triangles.push_back({i * nZ + j,
                                 (i + 1) * nZ + (j + 1),
                                 i * nZ + (j + 1)});
        }
    }

    for (std::vector<unsigned int> triangle : triangles)
    {
        indices.insert(indices.end(), triangle.begin(), triangle.end());
    }
}
/*******************************************************************************/

void loadTexture(char *filename, GLuint programID, char *varName)
{
    static int textureidx = 0;
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + textureidx);
    glBindTexture(GL_TEXTURE_2D, texture);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(programID, varName), textureidx);
    textureidx++;
}

/*******************************************************************************/

std::vector<unsigned int> indices; // Triangles concaténés dans une liste
std::vector<std::vector<unsigned int>> triangles;
std::vector<glm::vec3> indexed_vertices;
std::vector<glm::vec2> textures_coords;

int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Moteur - GLFW", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited mouvement
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    Mesh terrain;
    generate_scene(indices, triangles, indexed_vertices, textures_coords);
    terrain.setData(indexed_vertices, textures_coords, indices);
    // Load it into a VBO

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    loadTexture("Textures/heightmap-1024x1024.png", programID, "heightMap");
    // loadTexture("Textures/heightMap.png", programID, "heightMap");

    loadTexture("Textures/grass.png", programID, "grass");
    loadTexture("Textures/rock.png", programID, "rock");
    loadTexture("Textures/snowrocks.png", programID, "snowrocks");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do
    {

        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, terrain);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        glm::mat4 model = glm::mat4();

        glm::mat4 view = glm::lookAt(camera_position, camera_target, camera_up);

        glm::mat4 proj = glm::perspective(glm::radians(45.), ((double)SCR_WIDTH) / SCR_HEIGHT, 0.1, 100.);

        GLfloat modelLocation = glGetUniformLocation(programID, "model");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        GLfloat projLocation = glGetUniformLocation(programID, "proj");
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj));

        GLfloat viewLocation = glGetUniformLocation(programID, "view");
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

        terrain.draw();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

// ---------------------------------------------------------------------------------------------------------

void regenerateTerrain(Mesh &terrainMesh)
{
    indices.clear();
    triangles.clear();
    indexed_vertices.clear();
    textures_coords.clear();

    generate_scene(indices, triangles, indexed_vertices, textures_coords);
    terrainMesh.setData(indexed_vertices, textures_coords, indices);

    std::cout << "Terrain regenerated: " << nX << " x " << nZ << std::endl;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Mesh &terrain)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera zoom in and out
    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_position += cameraSpeed * camera_target;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_position -= cameraSpeed * camera_target;

    static bool plusPressed = false;
    static bool minusPressed = false;

    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    {
        if (!plusPressed)
        {
            nX *= 2;
            nZ *= 2;
            plusPressed = true;
            regenerateTerrain(terrain);
        }
    }
    else
    {
        plusPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    {
        if (!minusPressed)
        {
            nX = std::max(4, nX / 2);
            nZ = std::max(4, nZ / 2);
            minusPressed = true;
            regenerateTerrain(terrain);
        }
    }
    else
    {
        minusPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        nX = 0;
        nZ = 0;
        regenerateTerrain(terrain);
        std::cout << "delete terrain" << std::endl;
    }
    // TODO add translations
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
