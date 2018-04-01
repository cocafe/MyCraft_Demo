#version 330 core

// UV data passed by vertex shader
in vec2 uv;

// Output data accepted by OpenGL
out vec4 color;

// Texture handle
uniform sampler2D sampler;

void main()
{
    color = texture(sampler, uv).rgba;
}