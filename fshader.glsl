#version 150
in vec3 v_position;
in vec3 v_normal;
in float f_waterHeight;
in float f_groundHeight;
in vec2 f_texCooord;

out vec4 fragColor;

uniform vec3 light_position;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

void main() {

    vec4 reflectColor = texture2D(reflectionTexture,f_texCooord);
    vec4 refractColor = texture2D(refractionTexture,f_texCooord);


    fragColor = mix(reflectColor,refractColor,0.5);
}
