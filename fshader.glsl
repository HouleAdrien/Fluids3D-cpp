#version 150
in vec3 v_position;
in vec3 v_normal;
in float f_waterHeight;

out vec4 fragColor;

uniform vec3 light_position;

void main() {
    vec3 L = normalize(light_position - v_position);
    float NL = max(dot(normalize(v_normal), L), 0.0);
    float heightFactor = clamp(f_waterHeight / 10.0, 0.0, 1.0);
    vec3 colorGradient = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), heightFactor); // Créer un gradient
    vec3 col = clamp(colorGradient * 0.2 + colorGradient * 0.8 * NL, 0.0, 1.0);
    fragColor = vec4(col, 1.0);
}
