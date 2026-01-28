#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;
flat out int whichTexture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;
uniform sampler2D heightMap;

void main(){
    texCoord = aTexCoord;
    vec3 pos = vertices_position_modelspace;
    pos.y = texture(heightMap, texCoord).r - 0.2;
    gl_Position = projection * view * model * vec4(pos, 1);

    whichTexture = 0;

    if (pos.y > 0.2) {
        if (pos.y < 0.35) {
            whichTexture = 1;
        } else {
            whichTexture = 2;
        }
    }
}

