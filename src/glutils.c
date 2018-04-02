#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "debug.h"
#include "utils.h"
#include "glutils.h"

GLuint vertex_array_create(void)
{
        GLuint vertex_array;

        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        return vertex_array;
}

int vertex_array_delete(GLuint *vertex_array)
{
        if (!vertex_array)
                return -EINVAL;

        glDeleteVertexArrays(1, vertex_array);
        *vertex_array = 0;

        return 0;
}

GLuint buffer_create(void *data, GLsizei size)
{
        GLuint buffer;

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, GL_BUFFER_NONE);

        return buffer;
}

int buffer_delete(GLuint *buffer)
{
        if (!buffer)
                return -EINVAL;

        glDeleteBuffers(1, buffer);
        *buffer = GL_BUFFER_NONE;

        return 0;
}

GLuint shader_compile(GLenum type, const char *source)
{
        GLuint shader;
        GLint ret;
        GLint len;

        if (!source)
                return GL_SHADER_NONE;

        shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
                char *msg = buf_alloc((size_t)len);
                if (!msg) {
                        pr_err_alloc();
                        goto err_compile;
                }

                glGetShaderInfoLog(shader, len, NULL, msg);
                pr_err_func("%s\n", msg);
                buf_free(&msg);
        }

        glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
        if (ret != GL_TRUE) {
                pr_err_func("failed to compile shader\n");
                goto err_compile;
        }

        return shader;

err_compile:
        glDeleteShader(shader);

        return shader;
}

GLuint shader_load(GLenum type, const char *filepath)
{
        char *src;
        GLuint shader;

        src = file_read(filepath);
        if (!src) {
                return GL_SHADER_NONE;
        }

        shader = shader_compile(type, src);
        if (glIsShader(shader) != GL_TRUE) {
                pr_err_func("failed to load shader\n");
        }

        buf_free(&src);

        return shader;
}

int shader_delete(GLuint *shader)
{
        if (!shader)
                return -EINVAL;

        glDeleteShader(*shader);
        *shader = GL_SHADER_NONE;

        return 0;
}

GLuint program_link(GLuint shader_vertex, GLuint shader_frag)
{
        GLuint program;
        GLint ret;
        GLint len;

        program = glCreateProgram();
        glAttachShader(program, shader_vertex);
        glAttachShader(program, shader_frag);
        glLinkProgram(program);

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
                char *msg = buf_alloc((size_t)len);
                if (!msg) {
                        pr_err_alloc();
                        goto err_link;
                }

                glGetProgramInfoLog(program, len, NULL, msg);
                pr_err_func("%s\n", msg);
                buf_free(&msg);
        }

        glGetProgramiv(program, GL_LINK_STATUS, &ret);
        if (ret != GL_TRUE) {
                pr_err_func("failed to link program\n");
                goto err_link;
        }

        glDetachShader(program, shader_vertex);
        glDetachShader(program, shader_frag);

        return program;

err_link:
        glDetachShader(program, shader_vertex);
        glDetachShader(program, shader_frag);
        glDeleteProgram(program);

        return program;
}

GLuint program_create(const char *filepath_shader_vert,
                      const char *filepath_shader_frag)
{
        GLuint shader_vert;
        GLuint shader_frag;
        GLuint program = GL_PROGRAM_NONE;

        shader_vert = shader_load(GL_VERTEX_SHADER, filepath_shader_vert);
        if (glIsShader(shader_vert) == GL_FALSE) {
                pr_err_func("failed to load shader %s\n", filepath_shader_vert);
                goto err_vert;
        }

        shader_frag = shader_load(GL_FRAGMENT_SHADER, filepath_shader_frag);
        if (glIsShader(shader_frag) == GL_FALSE) {
                pr_err_func("failed to load shader %s\n", filepath_shader_frag);
                goto err_frag;
        }

        program = program_link(shader_vert, shader_frag);
        if (glIsProgram(program) == GL_FALSE) {
                pr_err_func("failed to link program\n");
                goto err_program;
        }

err_program:
        shader_delete(&shader_frag);
err_frag:
        shader_delete(&shader_vert);
err_vert:
        return program;
}

int program_delete(GLuint *program)
{
        if (!program)
                return -EINVAL;

        glDeleteProgram(*program);
        *program = 0;

        return 0;
}

GLuint texture_png_create(image_png *png, int32_t filter_level)
{
        GLuint texture;

        if (!png)
                return GL_TEXTURE_NONE;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // TODO: glTexStorage2D(), mipmap level
        glTexImage2D(GL_TEXTURE_2D, 0,
                     (png->bpp == PNG_BPP_RGB) ? GL_RGB : GL_RGBA,
                     png->width, png->height, 0,
                     (png->bpp == PNG_BPP_RGB) ? GL_RGB : GL_RGBA,
                     GL_UNSIGNED_BYTE, png->data);

        switch (filter_level) {
                default:
                case FILTER_NEAREST:
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);

                        break;

                case FILTER_LINEAR:
                case FILTER_LINEAR_MIPMAP_LINEAR:
                        glGenerateMipmap(GL_TEXTURE_2D);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MIN_FILTER,
                                        GL_LINEAR_MIPMAP_LINEAR);

                        break;
        }

        return texture;
}

int texture_delete(GLuint *texture)
{
        if (!texture)
                return -EINVAL;

        glDeleteTextures(1, texture);
        *texture = GL_TEXTURE_NONE;

        return 0;
}


/**
 * FPS Meter
 */

int fps_meter_init(fps_meter *fps)
{
        memzero(fps, sizeof(fps_meter));
        fps->frame_time_last = glfwGetTime();

        return 0;
}

void fps_meter_count(fps_meter *fps)
{
        fps->frame_count++;
}

void fps_meter_measure(fps_meter *fps)
{
        double sec_1 = 1.0;
        double ms_1000 = 1000.0;

        fps->frame_time_curr = glfwGetTime();

        if ((fps->frame_time_curr - fps->frame_time_last) >= sec_1) {
                // FIXME: If we render fps within other thread, this needs sync?
                fps->fps = fps->frame_count;
                fps->frame_time_ms = ms_1000 / (double)fps->frame_count;

                fps->frame_count = 0;
                fps->frame_time_last = glfwGetTime();
        }
}

void fps_meter_render(fps_meter *fps)
{
        // TODO: fps_meter_render()
}
