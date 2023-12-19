#version 150

in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 fragColor;

uniform vec3 light_position;

void main() {
    // Calculate the distance from the fragment's position to the center
    float radius = 0.8; // Adjust the radius as needed
    vec2 center = vec2(0.5, 0.5); // Center of the point (assuming a normalized coordinate system)

    vec2 fragPos = gl_PointCoord; // Use gl_PointCoord for point sprites

    float distanceToCenter = distance(fragPos, center);

    // Check if the fragment is within the circle
    if (distanceToCenter <= radius) {
        // Set the fragment color to blue if it's inside the circle
        fragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue color
    } else {
        // Set the fragment color to red if it's outside the circle
        fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
    }
}
