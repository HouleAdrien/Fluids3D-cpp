/* Vertex Shader */
#version 330 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 mvp_matrix;

void main()
{
    gl_Position = mvp_matrix * vec4(vertexPosition, 1.0);
}
