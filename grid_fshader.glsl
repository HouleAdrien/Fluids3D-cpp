#version 150

// Include the following line to declare the texture function
#extension GL_ARB_texture_query_lod : enable

in vec3 v_position;
in vec3 v_normal;
in float v_height; // Receive the height from the vertex shader

out vec4 fragColor;

uniform vec3 light_position;

// Code from the first shader (with #version 330)
// Comment out explicit location since it might not be supported
// layout(location = 0) out vec4 FragColor;

in vec4 Color;
in vec2 Tex;
in vec3 WorldPos;

uniform sampler2D gTextureHeight0;
uniform sampler2D gTextureHeight1;
uniform sampler2D gTextureHeight2;
uniform sampler2D gTextureHeight3;

uniform float gHeight0 = 64.0;
uniform float gHeight1 = 128.0;
uniform float gHeight2 = 193.0;
uniform float gHeight3 = 256.0;

vec4 CalcTexColor()
{
    vec4 TexColor;

    float Height = WorldPos.y;

    if (Height < gHeight0) {
        TexColor = texture2D(gTextureHeight0, Tex);
    } else if (Height < gHeight1) {
        vec4 Color0 = texture2D(gTextureHeight0, Tex);
        vec4 Color1 = texture2D(gTextureHeight1, Tex);
        float Delta = gHeight1 - gHeight0;
        float Factor = (Height - gHeight0) / Delta;
        TexColor = mix(Color0, Color1, Factor);
    } else if (Height < gHeight2) {
        vec4 Color0 = texture2D(gTextureHeight1, Tex);
        vec4 Color1 = texture2D(gTextureHeight2, Tex);
        float Delta = gHeight2 - gHeight1;
        float Factor = (Height - gHeight1) / Delta;
        TexColor = mix(Color0, Color1, Factor);
    } else if (Height < gHeight3) {
        vec4 Color0 = texture2D(gTextureHeight2, Tex);
        vec4 Color1 = texture2D(gTextureHeight3, Tex);
        float Delta = gHeight3 - gHeight2;
        float Factor = (Height - gHeight2) / Delta;
        TexColor = mix(Color0, Color1, Factor);
    } else {
        TexColor = texture2D(gTextureHeight3, Tex);
    }

    return TexColor;
}

void main()
{
    vec4 TexColor = CalcTexColor();

    // Use fragColor instead of FragColor
    fragColor = Color * TexColor;

    // Mountain gradient color calculation
    vec3 baseColor = vec3(0.0, 0.5, 0.0); // Greenish color for base
    vec3 peakColor = vec3(1.0, 1.0, 1.0); // Whitish color for peak
    float t = clamp(v_height, 0.0, 1.0); // Ensure height is between 0 and 1
    vec3 mountainColor = mix(baseColor, peakColor, t); // Interpolate between base and peak colors based on height

    // Lighting calculations
    vec3 L = normalize(light_position - v_position);
    float NL = max(dot(normalize(v_normal), L), 0.0);

    // Combine mountain gradient color with lighting effect
    vec3 litColor = mountainColor * NL; // Modulate the mountain color with the lighting

    // Assign to fragColor
    fragColor = vec4(litColor, 1.0);
}
