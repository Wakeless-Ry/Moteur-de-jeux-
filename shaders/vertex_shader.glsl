#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
flat out int textureChoice;

uniform mat4 proj, model, view; 
uniform sampler2D heightMap;

void main(){

        // TODO : Output position of the vertex, in clip space : MVP * position
        vec3 pos = vertices_position_modelspace;
        pos.y=texture(heightMap,aTexCoord).r;
        gl_Position = proj * view * model * vec4(pos,1);
        TexCoord = aTexCoord;

        textureChoice = 0;

        if (pos.y > 0.2) {
                if (pos.y < 0.35) {
                        textureChoice = 1;
                } else {
                        textureChoice = 2;
                }
        }
}

