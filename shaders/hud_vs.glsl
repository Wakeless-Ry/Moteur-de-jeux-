#version 330 core

layout(location = 0) in vec2 inPos;    
layout(location = 1) in vec4 inColor;

out vec4 Color;

void main() {
    Color = inColor;
    gl_Position = vec4(inPos, 0.0, 1.0);
    
}