#version 330 core
// in
in vec2 UV;
in float height;
// Ouput data
out vec4 color;

uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D snowTex;

void main(){
        // each text def by height
        float h1 = 0.3;
        float h2 = 0.6;
        // texture
        vec3 grass = texture(grassTex, UV).rgb;
        vec3 rock  = texture(rockTex,  UV).rgb;
        vec3 snow  = texture(snowTex,  UV).rgb;

        vec3 finalColor;
        if (height < h1) {
                finalColor = grass;
        } 
        else if (height < h2) {
                float t = (height - h1) / (h2 - h1);
                finalColor = mix(grass, rock, t);
        } 
        else {
                float t = (height - h2) / (1.0 - h2);
                finalColor = mix(rock, snow, clamp(t, 0.0, 1.0));
        }

        color = vec4(finalColor, 1.0);
}
