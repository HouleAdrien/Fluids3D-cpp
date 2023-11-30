#version 150

in vec4 vertex;
in vec3 normal;
in vec2 texCoord;

out vec3 v_position;
out vec3 v_normal;
out vec2 Tex;

uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;
uniform sampler2D heightMap;
uniform float maxHeight;

void main() {
    float height = texture2D(heightMap, texCoord).r;

    vec3 modifiedVertex = vec3(vertex.x, vertex.y + height * maxHeight, vertex.z);
    v_position = modifiedVertex.xyz;
    v_normal = normal_matrix * normal;
    Tex = texCoord;
    gl_Position = mvp_matrix * vec4(modifiedVertex, 1.0);
}
