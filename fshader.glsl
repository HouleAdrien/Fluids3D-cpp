#version 150
in vec3 v_position;
in vec3 v_normal;
in float f_waterHeight;
in float f_groundHeight;

out vec4 fragColor;

uniform vec3 light_position;

// Fragment Shader
void main() {
    vec3 L = normalize(light_position - v_position);
    float NL = max(dot(normalize(v_normal), L), 0.0);
    float heightFactor = clamp(f_waterHeight / 10.0, 0.0, 1.0);

    // Adjusted color gradient for water
    vec3 deepWaterColor = vec3(0.0, 0.0, 0.5);
    vec3 shallowWaterColor = vec3(0.0, 0.5, 1.0);
    vec3 colorGradient = mix(deepWaterColor, shallowWaterColor, heightFactor);

    vec3 col = clamp(colorGradient * 0.2 + colorGradient * 2 * NL, 0.0, 1.0);

    // Adjusted alpha for transparency
    float alpha = mix(0.6, 0.8, heightFactor); // More transparent in deep water
    fragColor = vec4(col, alpha);
}
