#version 150

in vec4 vertex;
in vec3 normal;
in vec2 texCoord;

out vec3 v_position;
out vec3 v_normal;
out float v_height; // Output the height for the fragment shader

uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;
uniform sampler2D heightMap;
uniform float maxHeight;

void main() {
    float height = texture2D(heightMap, texCoord).r;
    v_height = height; // Pass the height to the fragment shader

    vec3 modifiedVertex = vec3(vertex.x, vertex.y + height * maxHeight, vertex.z);
    v_position = modifiedVertex.xyz;
    v_normal = normal_matrix * normal;
    gl_Position = mvp_matrix * vec4(modifiedVertex, 1.0);
}
