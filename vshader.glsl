#version 150
in vec4 vertex;
in vec3 normal;
in float waterHeight;

out vec3 v_position;
out vec3 v_normal;
out float f_waterHeight;

uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

void main() {
    v_position = vertex.xyz;
    v_normal = normal_matrix * normal;
    f_waterHeight = waterHeight;

    gl_Position = mvp_matrix * vertex;
}
