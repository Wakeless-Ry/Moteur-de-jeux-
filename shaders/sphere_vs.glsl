#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec3 vertices_normals_modelspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

out vec3 normal_viewspace;

void main(){
    vec3 pos = vertices_position_modelspace;
    gl_Position = projection * view * model * vec4(pos, 1);

    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    normal_viewspace = normalMatrix * vertices_normals_modelspace;
}

