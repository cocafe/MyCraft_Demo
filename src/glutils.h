#ifndef MYCRAFT_DEMO_GLUTILS_H
#define MYCRAFT_DEMO_GLUTILS_H

#include "../lib/png_loader.h"

#define GL_VAO_NONE                     (0)
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

typedef struct gl_attr {
        GLuint  program;        // program
        GLuint  vertex;         // buffer
        GLuint  vertex_nrm;     // buffer
        GLuint  uv;             // buffer
        GLuint  texel;          // texture
        GLint   sampler;        // uniform
        GLint   mat_transform;  // uniform
        GLuint  buffer_1;
        GLuint  buffer_2;
        GLuint  buffer_3;
        GLint   uniform_1;
        GLint   uniform_2;
        GLint   uniform_3;
} gl_attr;

GLuint vertex_array_create(void);
int vertex_array_delete(GLuint *vertex_array);

GLuint buffer_create(void *data, GLsizei size);
int buffer_delete(GLuint *buffer);

GLuint shader_compile(GLenum type, const char *source);
GLuint shader_load(GLenum type, const char *filepath);
int shader_delete(GLuint *shader);

GLuint program_link(GLuint shader_vertex, GLuint shader_frag);
GLuint program_create(const char *filepath_shader_vert,
                      const char *filepath_shader_frag);
int program_delete(GLuint *program);

GLuint texture_png_create(image_png *png, int32_t filter_level);
int texture_delete(GLuint *texture);

/**
 * FPS Meter
 */

typedef struct fps_meter {
        int32_t fps;
        double  frame_time_ms;

        int32_t frame_count;
        double  frame_time_curr;
        double  frame_time_last;
} fps_meter;

int fps_meter_init(fps_meter *fps);
void fps_meter_count(fps_meter *fps);
void fps_meter_measure(fps_meter *fps);
void fps_meter_render(fps_meter *fps);

#endif //MYCRAFT_DEMO_GLUTILS_H
