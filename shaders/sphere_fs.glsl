#version 330 core

in vec3 normal_viewspace;
out vec3 color;

void main(){
    color = normalize(normal_viewspace) * 0.5 + 0.5;
}
