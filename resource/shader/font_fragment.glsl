#version 330 core

in vec2 UV;

out vec4 color;

uniform vec3 color_font;
uniform vec3 color_font_r;

uniform vec3 color_shadow;
uniform vec3 color_shadow_r;

uniform sampler2D sampler;

void main()
{
    vec4 pixel = texture(sampler, UV);
    vec3 eps = vec3(0.009, 0.009, 0.009);

    if (all(greaterThanEqual(pixel, vec4(color_font - eps, 1.0))) &&
        all(lessThanEqual(pixel, vec4(color_font + eps, 1.0))))
        pixel = vec4(color_font_r, 1.0);

    if (all(greaterThanEqual(pixel, vec4(color_shadow - eps, 1.0))) &&
        all(lessThanEqual(pixel, vec4(color_shadow + eps, 1.0))))
        pixel = vec4(color_shadow_r, 1.0);

    color = pixel;
}