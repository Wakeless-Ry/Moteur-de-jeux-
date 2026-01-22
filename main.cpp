// Include standard headers
#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "lib/shader.hpp"

using namespace std;
using ushort = unsigned short;
using uint = unsigned int;

void processInput(GLFWwindow *window);

// settings
const uint SCR_WIDTH = 800;
const uint SCR_HEIGHT = 600;

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

template <typename T> T random(T min, T max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  if constexpr (std::is_integral<T>::value) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(gen);
  } else if constexpr (std::is_floating_point<T>::value) {
    std::uniform_real_distribution<T> dist(min, max);
    return dist(gen);
  } else {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value,
        "randomBetween only supports integral or floating point types");
  }
}

vector<ushort> flatten(vector<vector<ushort>> &values) {
  vector<ushort> result;

  for (vector<ushort> value : values) {
    result.insert(result.end(), value.begin(), value.end());
  }

  return result;
}

void generateTerrain(vector<vector<ushort>> &triangles,
                     vector<glm::vec3> &indexed_vertices,
                     vector<glm::vec2> &textures_coords) {
  const ushort nombreVertices = 16;
  const ushort nombreCases = nombreVertices - 1;
  const float minX = -0.9;
  const float maxX = 0.9;
  const float minY = -0.9;
  const float maxY = 0.9;

  const float stepX = (maxX - minX) / nombreCases;
  const float stepY = (maxY - minY) / nombreCases;

  for (ushort i = 0; i < nombreVertices; i++) {
    for (ushort j = 0; j < nombreVertices; j++) {
      indexed_vertices.push_back(
          glm::vec3(i * stepX + minX, random(-0.2, 0.2), j * stepY + minY));
      textures_coords.push_back(glm::vec2(((float)i) / (nombreVertices - 1),
                                          ((float)j) / (nombreVertices - 1)));
    }
  }

  for (ushort i = 0; i < nombreCases; i++) {
    for (ushort j = 0; j < nombreCases; j++) {
      triangles.push_back({(ushort)(i * nombreVertices + j),
                           (ushort)(i * nombreVertices + (j + 1)),
                           (ushort)((i + 1) * nombreVertices + j)});

      triangles.push_back({(ushort)(i * nombreVertices + (j + 1)),
                           (ushort)((i + 1) * nombreVertices + (j + 1)),
                           (ushort)((i + 1) * nombreVertices + j)});
    }
  }
}

void buildScene(vector<ushort> &indices, vector<vector<ushort>> &triangles,
                vector<glm::vec3> &indexed_vertices,
                vector<glm::vec2> &textures_coords) {
  // string filename("chair.off");
  // loadOFF(filename, indexed_vertices, indices, triangles);

  generateTerrain(triangles, indexed_vertices, textures_coords);

  indices = flatten(triangles);
}

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

uint loadTexture(GLuint programID, uint VAO) {
  uint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  unsigned char *data =
      stbi_load("assets/rock.png", &width, &height, &nrChannels, 0);

  if (data != nullptr) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    cout << "Failed to load texture" << endl;
  }

  stbi_image_free(data);

  return texture;
}

int main(void) {
  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "TP1 - GLFW", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    getchar();
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
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
  glfwSetCursorPos(window, 1024. / 2, 768. / 2);

  // Dark blue background
  glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  // Cull triangles which normal is not towards the camera
  // glEnable(GL_CULL_FACE);

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Create and compile our GLSL program from the shaders
  GLuint programID =
      LoadShaders("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

  /*****************TODO***********************/
  // Get a handle for our "Model View Projection" matrices uniforms

  /****************************************/
  vector<ushort> indices; // Triangles concaténés dans une liste
  vector<vector<ushort>> triangles;
  vector<glm::vec3> indexed_vertices;
  vector<glm::vec2> textures_coords;

  buildScene(indices, triangles, indexed_vertices, textures_coords);

  // Load it into a VBO

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3),
               &indexed_vertices[0], GL_STATIC_DRAW);

  // Generate a buffer for the indices as well
  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ushort),
               &indices[0], GL_STATIC_DRAW);

  GLuint texturebuffer;
  glGenBuffers(1, &texturebuffer);
  glBindBuffer(GL_ARRAY_BUFFER, texturebuffer);
  glBufferData(GL_ARRAY_BUFFER, textures_coords.size() * sizeof(glm::vec2),
               &textures_coords[0], GL_STATIC_DRAW);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(1);

  // Get a handle for our "LightPosition" uniform
  glUseProgram(programID);
  GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

  uint textureID = loadTexture(programID, VertexArrayID);

  // For speed computation
  double lastTime = glfwGetTime();
  int nbFrames = 0;

  do {
    // Measure speed
    // per-frame time logic
    // --------------------
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    glm::mat4 model = glm::mat4();
    glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));

    glm::mat4 view = glm::lookAt(camera_position, camera_target, camera_up);
    glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1, GL_FALSE,
                       glm::value_ptr(view));

    glm::mat4 projection = glm::perspective(
        glm::radians(45.), ((double)SCR_WIDTH) / SCR_HEIGHT, 0.1, 100.);
    glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1,
                       GL_FALSE, glm::value_ptr(projection));

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0,        // attribute
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0,        // stride
                          (void *)0 // array buffer offset
    );

    glBindTexture(GL_TEXTURE_2D, textureID);
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(GL_TRIANGLES,      // mode
                   indices.size(),    // count
                   GL_UNSIGNED_SHORT, // type
                   (void *)0          // element array buffer offset
    );

    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Cleanup VBO and shader
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &elementbuffer);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Camera zoom in and out
  float cameraSpeed = 2.5 * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera_position += cameraSpeed * camera_target;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera_position -= cameraSpeed * camera_target;

  // TODO add translations
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}