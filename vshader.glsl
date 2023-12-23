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
out vec4 clipSpace;
out vec2 f_texCoord;
out vec3 toCameraVector;

uniform vec3 cameraPos;
uniform mat4 mvp_matrix;     // Model-View-Projection matrix
uniform mat3 normal_matrix;  // Normal matrix for transforming normals
uniform mat4 model_matrix;
const float tiling = 1.0;

void main() {
    v_position = vertex.xyz ;


    vec4 worldPos= model_matrix* vec4(v_position.x,v_position.y,v_position.z,1.0);

    clipSpace= mvp_matrix * vec4(v_position, 1.0);

    gl_Position = clipSpace;
    v_position =worldPos.xyz;
    v_normal = normalize(normal_matrix * normal);

    f_waterHeight = waterHeight;

    f_groundHeight = groundHeight;
    f_texCoord = vec2(vertex.x/2.0 +0.5,vertex.y/2.0+0.5)*tiling;

    toCameraVector = cameraPos * worldPos.xyz;
}
