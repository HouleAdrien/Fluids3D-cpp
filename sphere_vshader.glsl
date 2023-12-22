#version 400

in vec4 vertex;
in vec3 normal;
in vec2 texCoord;

out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragPosition; // Pass the world-space position to the fragment shader

uniform mat4 mvp_matrix;     // Model-View-Projection matrix
uniform mat3 normal_matrix;  // Normal matrix for transforming normals

uniform vec4 plane;

void main() {
    fragTexCoord = texCoord;

    fragNormal = normal_matrix * normal;

    // Calculate the world-space position of the vertex
    fragPosition = vertex.xyz ;

    gl_Position = mvp_matrix * vertex;
    gl_ClipDistance[0] = dot(vertex,plane);
}
