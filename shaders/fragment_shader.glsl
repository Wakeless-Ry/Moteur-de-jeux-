#version 330 core

// Ouput data
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform sampler2D heightMap;

void main()

{
    if(TexCoord.x>0.5){
        FragColor = texture(ourTexture, TexCoord);
    }
    else 
        FragColor = texture(heightMap, TexCoord);

}
