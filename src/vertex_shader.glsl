#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

void main(){
    texCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1);
}

