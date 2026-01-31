#version 330 core

// Ouput data
out vec4 FragColor;
  
in vec2 TexCoord;
flat in int textureChoice;

uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D snowrocks;

uniform sampler2D heightMap;

void main()
{
    if(textureChoice == 0){
        FragColor = texture(grass, TexCoord);
    }  else if(textureChoice == 1){
        FragColor = texture(rock, TexCoord);
    }else{
        FragColor = texture(snowrocks, TexCoord);
    }
}


