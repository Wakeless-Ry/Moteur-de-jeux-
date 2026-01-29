#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 proj, model, view; 


void main(){

        // TODO : Output position of the vertex, in clip space : MVP * position
        gl_Position = proj * view * model * vec4(vertices_position_modelspace,1);
        TexCoord = aTexCoord;
}

