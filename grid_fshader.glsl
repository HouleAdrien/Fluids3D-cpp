#version 150

// Include the following line to declare the texture function
#extension GL_ARB_texture_query_lod : enable

// Inputs from the vertex shader
in vec3 v_position;
in vec3 v_normal;
in vec2 Tex;

out vec4 fragColor;

// Uniforms
uniform vec3 light_position;
uniform float maxHeight;

uniform sampler2D gTextureHeight0;
uniform sampler2D gTextureHeight1;
uniform sampler2D gTextureHeight2;
uniform sampler2D gTextureHeight3;

vec4 CalcTexColor() {
    vec4 TexColor;
    float Height = v_position.y;

    // Define thresholds as percentages of maxHeight
    float heightThreshold1 = maxHeight * 0.05;
    float heightThreshold2 = maxHeight * 0.40;
    float heightThreshold3 = maxHeight * 0.55;

    if (Height < heightThreshold1) {
        TexColor = texture2D(gTextureHeight0, Tex);
    } else if (Height < heightThreshold2) {
        TexColor = mix(texture2D(gTextureHeight0, Tex), texture2D(gTextureHeight1, Tex), (Height - heightThreshold1) / (heightThreshold2 - heightThreshold1));
    } else if (Height < heightThreshold3) {
        TexColor = mix(texture2D(gTextureHeight1, Tex), texture2D(gTextureHeight2, Tex), (Height - heightThreshold2) / (heightThreshold3 - heightThreshold2));
    } else {
        TexColor = texture2D(gTextureHeight3, Tex);
    }

    return TexColor;
}


void main() {
    vec4 TexColor = CalcTexColor();

    vec3 L = normalize(light_position - v_position); // Direction to the light
    float NL = max(dot(normalize(v_normal), L), 0.0); // Diffuse lighting factor
    vec3 litColor = TexColor.rgb * NL;
    fragColor = vec4(litColor, 1);
}
