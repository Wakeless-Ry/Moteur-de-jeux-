// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <common/png.hpp>

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
unsigned short NB_triangles = 16;
int old_NB_triangles = NB_triangles;
std::vector<glm::vec3> planeVertices;
std::vector<unsigned short> planeIndices;
std::vector<glm::vec2> planeUVs;

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint elementbuffer;
// camera
glm::vec3 camera_position   = glm::vec3(0.0f, 13.0f,  0.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 0.0f,  -1.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 0.;
float zoom = 1.;

// heighmap load
int l, L, c;
unsigned char* data = nullptr;

/*******************************************************************************/

void generatePlane(
    int N,
    float size,
    std::vector<glm::vec3>& vertices,
    std::vector<unsigned short>& indices,
    std::vector<glm::vec2>& uvs,
    float maxHeight = 0.f
) {
    vertices.clear();
    indices.clear();
    uvs.clear();

    float step = size / (N - 1);

    for (int z = 0; z < N; z++) {
        for (int x = 0; x < N; x++) {
            // height
            int px = x * (l - 1) / (N - 1);
            int py = z * (L - 1) / (N - 1);
            unsigned char pixel = data[(py * l + px) * c];
            float height = (pixel / 255.0f) * maxHeight;
            //vertex
            float xpos = -size / 2 + x * step;
            float zpos = -size / 2 + z * step;
            // uv
            vertices.push_back(glm::vec3(xpos, height, zpos));
            uvs.push_back(glm::vec2(
                (float)x / (N - 1),
                (float)z / (N - 1)
            ));
        }
    }

    for (int z = 0; z < N - 1; z++) {
        for (int x = 0; x < N - 1; x++) {
            int i0 = z * N + x;
            int i1 = i0 + 1;
            int i2 = i0 + N;
            int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }
}

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP2 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
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
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // heightmap load
    data = stbi_load("../texture/heightmap-1024x1024.png", &l, &L, &c, 0);
    if (!data) {
        std::cerr << "Path ?" << std::endl;
        return -1;
    }
    // chargement texture 
    // GLuint textureID = loadDDS("../texture/pixelart.dds");
    GLuint grassTex = loadPNG("../texture/grass.png");
    GLuint rockTex  = loadPNG("../texture/rock.png");
    GLuint snowTex  = loadPNG("../texture/snowrocks.png");
    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms

    /****************************************/
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;

    //Chargement du fichier de maillage
    // std::string filename("chair.off");
    // loadOFF(filename, indexed_vertices, indices, triangles );
    generatePlane(NB_triangles, 10.f, planeVertices, planeIndices, planeUVs, 2.f);
    // Load it into a VBO

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER,  planeVertices.size() * sizeof(glm::vec3), &planeVertices[0], GL_STATIC_DRAW);

    // uv
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, planeUVs.size() * sizeof(glm::vec2), &planeUVs[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planeIndices.size() * sizeof(unsigned short), &planeIndices[0], GL_STATIC_DRAW);
    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{

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


        /*****************TODO***********************/
        // Model matrix : an identity matrix (model will be at the origin) then change

        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms

        /****************************************/
        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 View = glm::lookAt(camera_position, camera_target, camera_up);
        glm::mat4 Projection = glm::perspective(glm::radians(45.f), 4.f / 3.f, 0.1f, 100.f);
        glm::mat4 MVP = Projection * View * Model;
        GLuint MatrixID = glGetUniformLocation(programID, "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );
        // uv
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        glUniform1i(glGetUniformLocation(programID, "grassTex"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rockTex);
        glUniform1i(glGetUniformLocation(programID, "rockTex"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, snowTex);
        glUniform1i(glGetUniformLocation(programID, "snowTex"), 2);
        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    planeIndices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );
        

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
    stbi_image_free(data);
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void update()
    {
        NB_triangles = static_cast<unsigned short>(glm::clamp(static_cast<int>(NB_triangles), 2, 128));

        if (NB_triangles != old_NB_triangles) {
            printf("Nb triangles: %d\n", NB_triangles);
            old_NB_triangles = NB_triangles;

            generatePlane(NB_triangles, 10.f, planeVertices, planeIndices, planeUVs, 3.f);

            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(glm::vec3), &planeVertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, planeUVs.size() * sizeof(glm::vec2), &planeUVs[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, planeIndices.size() * sizeof(unsigned short), &planeIndices[0], GL_STATIC_DRAW);
        }
    }
unsigned short mode = 0;
float angleMode1 = 0.f;
float cameraSpeed = 0.5f;
float rotationSpeed = 0.3f;
float angleMode2 = 0.f;
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    static bool cPressedLastFrame = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cPressedLastFrame)
    {
        mode = (mode + 1) % 3;
        if (mode == 0) printf("Mode Libre\n");
        else if (mode == 1) printf("Angle 45° fixe\n");
        else printf("Mode Orbital\n");
        cPressedLastFrame = true;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
        cPressedLastFrame = false;
    // speed
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) 
    {
        rotationSpeed += 0.1f * deltaTime;
        cameraSpeed += 1.1f * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rotationSpeed -= 0.1f * deltaTime;
        cameraSpeed -= 1.1f * deltaTime;
    }
    cameraSpeed = glm::clamp(cameraSpeed, 0.1f, 3.f);
    rotationSpeed = glm::clamp(rotationSpeed, 0.1f, 3.f);
    // res
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    {
        NB_triangles += 1;
        update();
    }
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    {
        NB_triangles -= 1;
        update();
    }
    //TODO add translations
    if (mode == 0)
    {
        glm::vec3 direction = glm::normalize(camera_target - camera_position);
        glm::vec3 side = glm::normalize(glm::cross(direction, camera_up));
        glm::vec3 moveDir(0.f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += direction;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= direction;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += side;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= side;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) moveDir += camera_up;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) moveDir -= camera_up;
        if (glm::length(moveDir) > 0)
            camera_position += glm::normalize(moveDir) * cameraSpeed;
    } else if (mode == 1) {
        angleMode1 += rotationSpeed * deltaTime;
        float radius = 10.f;
        float height = 10.f;
        camera_position = glm::vec3(radius * sin(angleMode1), height, radius * cos(angleMode1));
        camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
        camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    } else {
        float radius = 20.f;
        float height = 20.f;
        angleMode2 += rotationSpeed * deltaTime;
        camera_position = glm::vec3(radius * sin(angleMode2), height, radius * cos(angleMode2));
        camera_target = glm::vec3(0.f, 0.f, 0.f);
        camera_up = glm::vec3(0.f, 1.f, 0.f);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
