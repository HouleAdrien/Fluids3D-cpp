#version 150

in vec4 vertex;          // The original vertex position
in vec3 normal;          // The normal at the vertex
in vec2 texCoord;

in float waterHeight;    // Height of the water at the vertex, from your fluid motion computation
in float groundHeight;

out vec3 v_position;     // Passed to the fragment shader: position of the vertex
out vec3 v_normal;       // Passed to the fragment shader: normal of the vertex after applying wave effect
out float f_waterHeight; // Passed to the fragment shader: height of the water
out float f_groundHeight;
out vec2 f_texCooord;

uniform mat4 mvp_matrix;     // Model-View-Projection matrix
uniform mat3 normal_matrix;  // Normal matrix for transforming normals

void main() {
    // Pass the displaced position to the fragment shader
    v_position = vertex.xyz ;

    // Compute the transformed position using the MVP matrix
    gl_Position = mvp_matrix * vec4(v_position, 1.0);

    // Calculate the new normal after applying the wave motion
    // Here we're just passing the original normal, but you can modify this
    // to reflect the changes in the surface due to waves
    v_normal = normal_matrix * normal;

    // Pass the water height to the fragment shader
    f_waterHeight = waterHeight;

    f_groundHeight = groundHeight;

    f_texCooord = texCoord;
}
