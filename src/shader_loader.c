#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <GL/glew.h>

#include "debug.h"
#include "util.h"
#include "shader_loader.h"

int shaders_load(GLuint *program_id, const char *vertex_shader_path,
                 const char *frag_shader_path)
{
        GLuint vertex_shader_id;
        GLuint frag_shader_id;
        GLint ret = GL_FALSE;
        GLint info_log_len;

        if (!program_id || !vertex_shader_path || !frag_shader_path)
                return -EINVAL;

        char *buf_vertex = file_read(vertex_shader_path);
        if (!buf_vertex) {
                pr_err_func("failed to load shader %s\n", vertex_shader_path);
                ret = -EIO;
                goto err_buf_vertex;
        }

        char *buf_frag = file_read(frag_shader_path);
        if (!buf_frag) {
                pr_err_func("failed to load shader %s\n", frag_shader_path);
                ret = -EIO;
                goto err_buf_frag;
        }

        // TODO: use array to reduce compile shader code length

        vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        pr_debug_func("compile shader: %s\n", vertex_shader_path);
        glShaderSource(vertex_shader_id, 1, (const char **)&buf_vertex, NULL);
        glCompileShader(vertex_shader_id);

        glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &ret);
        glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_len);
        if (info_log_len > 0) {
                char *msg = buf_alloc((size_t)info_log_len + 2);
                if (!msg) {
                        pr_err_alloc();
                        ret = -ENOMEM;
                        goto err_compile;
                }

                glGetShaderInfoLog(vertex_shader_id, info_log_len, NULL, msg);
                pr_err_func("%s\n", msg);
                free(msg);
        }
        if (ret != GL_TRUE) {
                pr_err_func("failed to compile: %s\n", vertex_shader_path);
                goto err_compile;
        }

        pr_debug_func("compile shader: %s\n", frag_shader_path);
        glShaderSource(frag_shader_id, 1, (const char **)&buf_frag, NULL);
        glCompileShader(frag_shader_id);

        glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &ret);
        glGetShaderiv(frag_shader_id, GL_INFO_LOG_LENGTH, &info_log_len);
        if (info_log_len > 0) {
                char *msg = buf_alloc((size_t)info_log_len + 2);
                if (!msg) {
                        pr_err_alloc();
                        ret = -ENOMEM;
                        goto err_compile;
                }

                glGetShaderInfoLog(frag_shader_id, info_log_len, NULL, msg);
                pr_err_func("%s\n", msg);
                free(msg);
        }
        if (ret != GL_TRUE) {
                pr_err_func("failed to compile: %s\n", frag_shader_path);
                goto err_compile;
        }

        pr_debug_func("link shaders\n");
        *program_id = glCreateProgram();
        glAttachShader(*program_id, vertex_shader_id);
        glAttachShader(*program_id, frag_shader_id);
        glLinkProgram(*program_id);

        glGetProgramiv(*program_id, GL_LINK_STATUS, &ret);
        glGetProgramiv(*program_id, GL_INFO_LOG_LENGTH, &info_log_len);
        if (info_log_len > 0) {
                char *msg = buf_alloc((size_t)info_log_len + 2);
                if (!msg) {
                        pr_err_alloc();
                        ret = -ENOMEM;
                        goto err_link;
                }

                glGetProgramInfoLog(*program_id, info_log_len, NULL, msg);
                pr_err_func("%s\n", msg);
                free(msg);
        }
        if (ret != GL_TRUE) {
                pr_err_func("failed to link program\n");
                goto err_link;
        }

err_link:
        glDetachShader(*program_id, vertex_shader_id);
        glDetachShader(*program_id, frag_shader_id);

err_compile:
        glDeleteShader(vertex_shader_id);
        glDeleteShader(frag_shader_id);

        free(buf_frag);
err_buf_frag:
        free(buf_vertex);

err_buf_vertex:

        return ret;
}
