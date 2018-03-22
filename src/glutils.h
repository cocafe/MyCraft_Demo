#ifndef MYCRAFT_DEMO_GLUTILS_H
#define MYCRAFT_DEMO_GLUTILS_H

#define GL_SHADER_NONE                  (0)
#define GL_BUFFER_NONE                  (0)
#define GL_SAMPLER_NONE                 (0)
#define GL_TEXTURE_NONE                 (0)
#define GL_PROGRAM_NONE                 (0)

GLuint shader_compile(GLenum type, const char *source);
GLuint shader_load(GLenum type, const char *filepath);
GLuint program_link(GLuint shader_vertex, GLuint shader_frag);
GLuint program_create(const char *filepath_shader_vert,
                      const char *filepath_shader_frag);

#endif //MYCRAFT_DEMO_GLUTILS_H
