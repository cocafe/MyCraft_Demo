#ifndef MYCRAFT_DEMO_GLUTILS_H
#define MYCRAFT_DEMO_GLUTILS_H

#include "../lib/png_loader.h"

#include "utils.h"

#define GL_VAO_NONE                     (0)
#define GL_SHADER_NONE                  (0)
#define GL_BUFFER_NONE                  (0)
#define GL_SAMPLER_NONE                 (0)
#define GL_TEXTURE_NONE                 (0)
#define GL_PROGRAM_NONE                 (0)

typedef enum texel_filter {
        FILTER_NEAREST = 0,
        FILTER_NEAREST_MIPMAP_NEAREST,
        FILTER_NEAREST_MIPMAP_LINEAR,
        FILTER_LINEAR,
        FILTER_LINEAR_MIPMAP_NEAREST,
        FILTER_LINEAR_MIPMAP_LINEAR,
        FILTER_ANISOTROPIC,     // TODO: EXT_texture_filter_anisotropic
        NR_TEXEL_FILTER,
} texel_filter;

typedef struct vertex_attr {
        vec3 position;
        vec3 normal;
        vec2 uv;
} vertex_attr;

typedef struct gl_attr {
        GLuint  program;        // program
        GLuint  vbo_index;      // buffer
        GLuint  vertex;         // buffer
        GLuint  vertex_uv;      // buffer
        GLuint  vertex_nrm;     // buffer
        GLsizei vertex_count;
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

void APIENTRY opengl_debug_output_callback(GLenum source, GLenum type,
                                           GLuint id, GLenum severity,
                                           GLsizei length,
                                           const GLchar *message,
                                           const void *userParam);

GLuint vertex_array_create(void);
int vertex_array_delete(GLuint *vertex_array);

GLuint buffer_create(void *data, GLsizeiptr size);
GLuint buffer_element_create(void *data, GLsizeiptr size);
int buffer_delete(GLuint *buffer);

GLuint shader_compile(GLenum type, const char *source);
GLuint shader_load(GLenum type, const char *filepath);
int shader_delete(GLuint *shader);

GLuint program_link(GLuint shader_vertex, GLuint shader_frag);
GLuint program_create(const char *filepath_shader_vert,
                      const char *filepath_shader_frag);
int program_delete(GLuint *program);

GLuint texture_png_create(image_png *png, int32_t filter_level, uint32_t mipmaps);
int texture_delete(GLuint *texture);

int gl_attr_init(gl_attr *attr);
int gl_attr_buffer_delete(gl_attr *attr);

int gl_vertices_alloc(vec3 **positions, vec3 **normals, vec2 **uvs, size_t count);
void gl_vertices_free(vec3 **positions, vec3 **normals, vec2 **uvs);

int text_string_draw(const char *str, int x, int y, float scale, int background,
                     int fb_width, int fb_height);
int text_render_init(void);
int text_render_deinit(void);

int crosshair_textured_draw(float scale, int fb_width, int fb_height);
int crosshair_textured_init(void);
int crosshair_textured_deinit(void);

/**
 * VBOs
 */

#define GL_VBO_ENABLED                  (1)
#define GL_VBO_DISABLED                 (0)

typedef struct gl_vbo {
        seqlist indices;
        seqlist vbo_attrs;
} gl_vbo;

int gl_vbo_init(gl_vbo *vbo);
int gl_vbo_deinit(gl_vbo *vbo);

int gl_vbo_index(gl_vbo *vbo, vertex_attr *vertices, uint32_t vertex_count);

int gl_vbo_buffer_create(gl_vbo *vbo, gl_attr *glattr);

int gl_vbo_is_empty(gl_vbo *vbo);

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
void fps_meter_draw(fps_meter *fps, int fb_width, int fb_height);

#endif //MYCRAFT_DEMO_GLUTILS_H
