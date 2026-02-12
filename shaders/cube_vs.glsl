#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

void main(){
    vec3 pos = vertices_position_modelspace;
    gl_Position = projection * view * model * vec4(pos, 1);
}

