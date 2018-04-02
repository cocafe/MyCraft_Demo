#version 330 core

// Parameters transfered by glEnableVertexAttribArray()
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

// UV data needed by fragment shader
out vec2 uv;

// Perspective transform matrix
uniform mat4 mat_transform;

void main() {
    gl_Position = mat_transform * vec4(vertex_position, 1);

    uv = vertex_uv;
}