#version 150

in vec3 v_position;
in vec3 v_normal;
in float v_height; // Receive the height from the vertex shader

out vec4 fragColor;

uniform vec3 light_position;

void main() {
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

    fragColor = vec4(litColor, 1.0);
}
