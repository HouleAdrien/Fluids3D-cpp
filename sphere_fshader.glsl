#version 400

in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragPosition;

out vec4 fragColor;

uniform vec3 light_position;
uniform float time;  // Time variable for animation

void main() {
    vec3 L = normalize(light_position - fragPosition);
    float NL = max(dot(normalize(fragNormal), L), 0.0);

    // Define a color animation using sine and cosine functions
    float red = 0.5 + 0.5 * sin(time);
    float green = 0.5 + 0.5 * cos(time);
    float blue = 0.5 + 0.5 * sin(time * 0.5);

    vec3 color = vec3(red, green, blue);

    vec3 litColor = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);
    fragColor = vec4(litColor, 1.0);
}
