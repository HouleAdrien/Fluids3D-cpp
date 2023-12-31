#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 mvp_matrix; // Model-View-Projection matrix

void main() {
    gl_Position = mvp_matrix * vec4(position, 1.0);
}
