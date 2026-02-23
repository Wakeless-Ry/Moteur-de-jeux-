#version 330 core

in vec3 normal_viewspace;
out vec4 color;
uniform sampler2D grassTexture;
in vec2 texcoord;

void main(){
    // color = normalize(normal_viewspace) * 0.5 + 0.5;
    color = texture(grassTexture, texcoord);
}
