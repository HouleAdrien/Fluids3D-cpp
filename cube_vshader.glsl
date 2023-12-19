#version 150

in vec4 vertex;
in vec3 normal;
in vec2 texCoord;

out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragPosition; // Pass the world-space position to the fragment shader

uniform mat4 mvp_matrix;     // Model-View-Projection matrix
uniform mat3 normal_matrix;  // Normal matrix for transforming normals

void main() {
    gl_Position = mvp_matrix * vertex;
    fragNormal = normal;
    fragTexCoord = texCoord;

    // Calculate the world-space position of the vertex
    fragPosition = vertex.xyz;


    // Pass the vertex position as a varying attribute
    gl_PointSize = 10.0f; // Adjust the point size as needed
}
