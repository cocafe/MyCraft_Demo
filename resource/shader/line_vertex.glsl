#version 330 core

layout(location = 0) in vec3 vertex_position;

out vec4 fragment_color;

uniform vec4 line_color;
uniform mat4 mat_transform;

void main() {
    fragment_color = line_color;
    gl_Position = mat_transform * vec4(vertex_position, 1);
}
