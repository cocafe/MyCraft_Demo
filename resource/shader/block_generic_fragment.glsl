#version 330 core

// UV data passed by vertex shader
in vec2 uv;

// Fog
in float fog_factor;
uniform vec4 fog_color;

// Output data accepted by OpenGL
out vec4 color;

// Texture handle
uniform sampler2D sampler;

void main()
{
    vec4 pixel = texture(sampler, uv).rgba;

    color = mix(pixel, fog_color, fog_factor);
}