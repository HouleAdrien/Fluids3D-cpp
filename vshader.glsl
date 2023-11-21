#version 150

in vec4 vertex;          // The original vertex position
in vec3 normal;          // The normal at the vertex
in float waterHeight;    // Height of the water at the vertex, from your fluid motion computation

out vec3 v_position;     // Passed to the fragment shader: position of the vertex
out vec3 v_normal;       // Passed to the fragment shader: normal of the vertex after applying wave effect
out float f_waterHeight; // Passed to the fragment shader: height of the water

uniform mat4 mvp_matrix;     // Model-View-Projection matrix
uniform mat3 normal_matrix;  // Normal matrix for transforming normals

void main() {
    // Apply the wave motion to the vertex position
    vec3 displacedPosition = vertex.xyz + normal * waterHeight;

    // Compute the transformed position using the MVP matrix
    gl_Position = mvp_matrix * vec4(displacedPosition, 1.0);

    // Pass the displaced position to the fragment shader
    v_position = displacedPosition;

    // Calculate the new normal after applying the wave motion
    // Here we're just passing the original normal, but you can modify this
    // to reflect the changes in the surface due to waves
    v_normal = normal_matrix * normal;

    // Pass the water height to the fragment shader
    f_waterHeight = waterHeight;
}
