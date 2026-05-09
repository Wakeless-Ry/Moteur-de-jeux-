#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

class Hud {
  public:
    void init(const char *vsHud = "shaders/hud_vs.glsl",
              const char *fsHud = "shaders/hud_fs.glsl");

    void beginFrame(int screenWidth, int screenHeight);
    void drawCollectibleCounter(int collected, int total);
    void endFrame();
    void cleanup();

  private:
    struct Vertex {
        float x, y;
        float r, g, b, a;
    };

    GLuint programId = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    int w = 1;
    int h = 1;

    std::vector<Vertex> vertices;

    void pushRectPx(float x, float y, float width, float height, float r,
                    float g, float b, float a);

    void drawDigit7Seg(int digit, int x, int y, int heightPx, int thicknessPx,
                       float r, float g, float b, float a);

    int drawNumber(int value, int x, int y, int heightPx, int thicknessPx,
                   int spacingPx, float r, float g, float b, float a);

    int drawSlash(int x, int y, int heightPx, int thicknessPx, int spacingPx,
                  float r, float g, float b, float a);
};
