#include "Hud.h"

#include <algorithm>
#include <cmath>

#include "lib/shader.hpp"

void Hud::init(const char *vsHud, const char *fsHud) {
    programId = LoadShaders(vsHud, fsHud);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, r));

    glBindVertexArray(0);
}

void Hud::beginFrame(int screenWidth, int screenHeight) {
    w = std::max(1, screenWidth);
    h = std::max(1, screenHeight);
    vertices.clear();
}

void Hud::endFrame() {
    if (programId == 0 || vao == 0 || vbo == 0)
        return;
    if (vertices.empty())
        return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(programId);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
}

void Hud::cleanup() {
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
    if (programId)
        glDeleteProgram(programId);
    vbo = 0;
    vao = 0;
    programId = 0;
}

/*Création d'un rectangle 2d en pixels écran*/
void Hud::pushRectPx(float x, float y, float width, float height, float r,
                     float g, float b, float a) {

    float x0 = (2.0f * (x / (float)w)) - 1.0f;
    float y0 = 1.0f - (2.0f * (y / (float)h));
    float x1 = (2.0f * ((x + width) / (float)w)) - 1.0f;
    float y1 = 1.0f - (2.0f * ((y + height) / (float)h));

    Vertex v0{x0, y0, r, g, b, a};
    Vertex v1{x0, y1, r, g, b, a};
    Vertex v2{x1, y0, r, g, b, a};
    Vertex v3{x1, y1, r, g, b, a};

    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);

    vertices.push_back(v2);
    vertices.push_back(v1);
    vertices.push_back(v3);
}

/* dessiner un chiffre comme les réveils digitals*/
void Hud::drawDigit7Seg(int digit, int x, int y, int heightPx, int thicknessPx,
                        float r, float g, float b, float a) {
    digit = std::clamp(digit, 0, 9);

    // 7 segments:
    // 0=top,1=top-left,2=top-right,3=mid,4=bot-left,5=bot-right,6=bot
    static const bool seg[10][7] = {
        {true, true, true, false, true, true, true},     // 0
        {false, false, true, false, false, true, false}, // 1
        {true, false, true, true, true, false, true},    // 2
        {true, false, true, true, false, true, true},    // 3
        {false, true, true, true, false, true, false},   // 4
        {true, true, false, true, false, true, true},    // 5
        {true, true, false, true, true, true, true},     // 6
        {true, false, true, false, false, true, false},  // 7
        {true, true, true, true, true, true, true},      // 8
        {true, true, true, true, false, true, true}      // 9
    };

    int hPx = heightPx;
    int wPx = (int)std::round(heightPx * 0.6f);
    int t = std::max(1, thicknessPx);

    int half = hPx / 2;
    int midY = y + half - t / 2;

    auto top = [&] {
        pushRectPx((float)(x + t), (float)y, (float)(wPx - 2 * t), (float)t, r,
                   g, b, a);
    };
    auto middle = [&] {
        pushRectPx((float)(x + t), (float)midY, (float)(wPx - 2 * t), (float)t,
                   r, g, b, a);
    };
    auto bottom = [&] {
        pushRectPx((float)(x + t), (float)(y + hPx - t), (float)(wPx - 2 * t),
                   (float)t, r, g, b, a);
    };

    int vTopH = std::max(1, half - (int)(1.5f * t));
    int vBotH = vTopH;

    auto topLeft = [&] {
        pushRectPx((float)x, (float)(y + t), (float)t, (float)vTopH, r, g, b,
                   a);
    };
    auto topRight = [&] {
        pushRectPx((float)(x + wPx - t), (float)(y + t), (float)t, (float)vTopH,
                   r, g, b, a);
    };
    auto bottomLeft = [&] {
        pushRectPx((float)x, (float)(y + half + t / 2), (float)t, (float)vBotH,
                   r, g, b, a);
    };
    auto bottomRight = [&] {
        pushRectPx((float)(x + wPx - t), (float)(y + half + t / 2), (float)t,
                   (float)vBotH, r, g, b, a);
    };

    if (seg[digit][0])
        top();
    if (seg[digit][1])
        topLeft();
    if (seg[digit][2])
        topRight();
    if (seg[digit][3])
        middle();
    if (seg[digit][4])
        bottomLeft();
    if (seg[digit][5])
        bottomRight();
    if (seg[digit][6])
        bottom();
}

/*Enchainement de plusieurs chiffres*/
int Hud::drawNumber(int value, int x, int y, int heightPx, int thicknessPx,
                    int spacingPx, float r, float g, float b, float a) {
    value = std::max(0, value);

    std::string s = std::to_string(value);
    int wDigit = (int)std::round(heightPx * 0.6f);
    int cursor = x;

    for (char c : s) {
        int d = c - '0';
        drawDigit7Seg(d, cursor, y, heightPx, thicknessPx, r, g, b, a);
        cursor += wDigit + spacingPx;
    }

    return cursor;
}

/*Le backslash*/
int Hud::drawSlash(int x, int y, int heightPx, int thicknessPx, int spacingPx,
                   float r, float g, float b, float a) {
    int t = std::max(1, thicknessPx);
    int n = 10;
    float stepX = (heightPx * 0.6f) / n;
    float stepY = (float)heightPx / n;

    for (int i = 0; i <= n; ++i) {
        float px = x + i * stepX;
        float py = y + (n - i) * stepY;
        pushRectPx(px, py, (float)t, (float)t, r, g, b, a);
    }

    int slashAdvance = (int)std::round(heightPx * 0.6f) + spacingPx;
    return x + slashAdvance;
}

/*Compostion de toutes les fonctions */
void Hud::drawCollectibleCounter(int collected, int total) {
    const int marginX = 20;
    const int marginY = 20;

    const int heightPx = 32;
    const int thicknessPx = 4;
    const int spacingPx = 6;

    const float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

    int x = marginX;
    int y = marginY;

    x = drawNumber(collected, x, y, heightPx, thicknessPx, spacingPx, r, g, b,
                   a);
    x = drawSlash(x, y, heightPx, thicknessPx, spacingPx, r, g, b, a);
    (void)drawNumber(total, x, y, heightPx, thicknessPx, spacingPx, r, g, b, a);
}