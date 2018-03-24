#ifndef MYCRAFT_DEMO_GLUTILS_H
#define MYCRAFT_DEMO_GLUTILS_H

#include "../lib/png_loader.h"

#define GL_SHADER_NONE                  (0)
#define GL_BUFFER_NONE                  (0)
#define GL_SAMPLER_NONE                 (0)
#define GL_TEXTURE_NONE                 (0)
#define GL_PROGRAM_NONE                 (0)

typedef enum texel_filter {
        FILTER_NEAREST = 0,
        FILTER_LINEAR,
        FILTER_LINEAR_MIPMAP_LINEAR,
        FILTER_ANISOTROPIC,     // TODO: EXT_texture_filter_anisotropic
        NR_TEXEL_FILTER,
} texel_filter;

GLuint shader_compile(GLenum type, const char *source);
GLuint shader_load(GLenum type, const char *filepath);
GLuint program_link(GLuint shader_vertex, GLuint shader_frag);
GLuint program_create(const char *filepath_shader_vert,
                      const char *filepath_shader_frag);
GLuint texture_png_create(image_png *png, int32_t filter_level);

#endif //MYCRAFT_DEMO_GLUTILS_H
