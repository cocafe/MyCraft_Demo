#version 330 core

layout(location = 0) in vec3 vertex_position_modelspace;
layout(location = 1) in vec3 vertex_color;

uniform mat4 MVP;

out vec3 fragment_color;

void main() {
    gl_Position = MVP * vec4(vertex_position_modelspace, 1);

    fragment_color = vertex_color;
}