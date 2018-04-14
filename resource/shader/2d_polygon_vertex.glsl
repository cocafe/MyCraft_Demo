#version 330 core

layout(location = 0) in vec2 vertex_xy; // Screen space
layout(location = 1) in vec2 vertex_uv;

out vec2 UV;

// (width, height)
uniform vec2 screen_size;

void main()
{
    vec2 screen_size_2 = screen_size / 2;

    // map [0 ... width][0 ... height] to [-1 ... 1][-1 ... 1] (NDC space)
    vec2 vertex_homo = vertex_xy - screen_size_2;
    vertex_homo /= screen_size_2;

    // (x, y, z, w)
    gl_Position = vec4(vertex_homo, 0, 1);

    UV = vertex_uv;
}