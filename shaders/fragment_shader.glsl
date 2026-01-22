#version 330 core

in vec2 texCoord;
flat in int whichTexture;

out vec4 color;

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;

void main(){
    if (whichTexture == 0) { 
        color = texture(grassTexture, texCoord);
    } else if (whichTexture == 1) {
        color = texture(rockTexture, texCoord);
    } else {
        color = texture(snowTexture, texCoord);
    }
}
