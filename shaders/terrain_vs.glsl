#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec3 vertices_normals_modelspace;
layout (location = 2) in vec2 vertices_texcoords;

uniform sampler2D heightMap;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

out vec2 TexCoords;

void main()
{
    TexCoords = vertices_texcoords;

    vec4 worldPos = model * vec4(vertices_position_modelspace, 1.0);
    worldPos.y = texture(heightMap, TexCoords).r;

    gl_Position = projection * view * worldPos;
}

