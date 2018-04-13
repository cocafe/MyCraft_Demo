#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <errno.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "debug.h"
#include "utils.h"
#include "glutils.h"

void APIENTRY opengl_debug_output_callback(GLenum source, GLenum type,
                                           GLuint id, GLenum severity,
                                           GLsizei length,
                                           const GLchar *message,
                                           const void *userParam)
{
        UNUSED_PARAM(id);
        UNUSED_PARAM(length);
        UNUSED_PARAM(userParam);

        pr_info("OpenGL Debug Output: ");

        if(source == GL_DEBUG_SOURCE_API_ARB)
                pr_info("[API]");
        else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
                pr_info("[WINDOW_SYSTEM]");
        else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
                pr_info("[SHADER_COMPILER]");
        else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
                pr_info("[THIRD_PARTY]");
        else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)
                pr_info("[APPLICATION]");
        else if(source == GL_DEBUG_SOURCE_OTHER_ARB)
                pr_info("[OTHER]");

        if(type == GL_DEBUG_TYPE_ERROR_ARB)
                pr_info("[ERROR]");
        else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
                pr_info("[DEPRECATED_BEHAVIOR]");
        else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
                pr_info("[UNDEFINED_BEHAVIOR]");
        else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)
                pr_info("[PORTABILITY]");
        else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
                pr_info("[PERFORMANCE]");
        else if(type == GL_DEBUG_TYPE_OTHER_ARB)
                pr_info("[OTHER]");

        if(severity == GL_DEBUG_SEVERITY_HIGH_ARB)
                pr_info("[HIGH]");
        else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
                pr_info("[MEDIUM]");
        else if(severity == GL_DEBUG_SEVERITY_LOW_ARB)
                pr_info("[LOW]");

        pr_info("MSG: %s\n", message);
}

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

GLuint buffer_create(void *data, GLsizeiptr size)
{
        GLuint buffer;

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, GL_BUFFER_NONE);

        return buffer;
}

GLuint buffer_element_create(void *data, GLsizeiptr size)
{
        GLuint buffer;

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

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

GLuint texture_png_create(image_png *png, int32_t filter_level, uint32_t mipmaps)
{
        GLuint texture;

        if (!png)
                return GL_TEXTURE_NONE;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (!mipmaps) {
                glTexImage2D(GL_TEXTURE_2D, 0,
                             (png->bpp == PNG_BPP_RGB) ? GL_RGB : GL_RGBA,
                             png->width, png->height, 0,
                             (png->bpp == PNG_BPP_RGB) ? GL_RGB : GL_RGBA,
                             GL_UNSIGNED_BYTE, png->data);
        } else {
                glTexStorage2D(GL_TEXTURE_2D, mipmaps, GL_RGBA8, png->width,
                               png->height);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, png->width, png->height,
                                (png->bpp == PNG_BPP_RGB) ? GL_RGB : GL_RGBA,
                                GL_UNSIGNED_BYTE, png->data);

                glGenerateMipmap(GL_TEXTURE_2D);
        }

//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        switch (filter_level) {
                default:
                case FILTER_NEAREST:
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        break;

                case FILTER_NEAREST_MIPMAP_NEAREST:
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                        break;

                case FILTER_NEAREST_MIPMAP_LINEAR:
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                        break;

                case FILTER_LINEAR:
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        break;

                case FILTER_LINEAR_MIPMAP_NEAREST:
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                        break;

                case FILTER_LINEAR_MIPMAP_LINEAR:
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

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

int gl_attr_init(gl_attr *attr)
{
        if (!attr)
                return -EINVAL;

        memzero(attr, sizeof(gl_attr));

        return 0;
}

int gl_attr_buffer_delete(gl_attr *attr)
{
        if (!attr)
                return -EINVAL;

        if (glIsBuffer(attr->vbo_index) != GL_FALSE)
                buffer_delete(&attr->vbo_index);

        if (glIsBuffer(attr->vertex) != GL_FALSE)
                buffer_delete(&attr->vertex);

        if (glIsBuffer(attr->vertex) != GL_FALSE)
                buffer_delete(&attr->vertex_nrm);

        if (glIsBuffer(attr->vertex_uv) != GL_FALSE)
                buffer_delete(&attr->vertex_uv);

        if (glIsBuffer(attr->buffer_1) != GL_FALSE)
                buffer_delete(&attr->buffer_1);

        if (glIsBuffer(attr->buffer_2) != GL_FALSE)
                buffer_delete(&attr->buffer_2);

        if (glIsBuffer(attr->buffer_3) != GL_FALSE)
                buffer_delete(&attr->buffer_3);

        return 0;
}

int gl_vertices_alloc(vec3 **positions, vec3 **normals, vec2 **uvs, size_t count)
{
        if (!positions || !normals || !uvs)
                return -EINVAL;

        *positions = memalloc(sizeof(vec3) * count);
        if (!*positions) {
                pr_err_alloc();
                return -ENOMEM;
        }

        *normals = memalloc(sizeof(vec3) * count);
        if (!*normals) {
                pr_err_alloc();
                goto free_vertex;
        }

        *uvs = memalloc(sizeof(vec2) * count);
        if (!*uvs) {
                pr_err_alloc();
                goto free_normal;
        }

        return 0;

free_vertex:
        memfree((void **)positions);

free_normal:
        memfree((void **)normals);

        return -ENOMEM;
}

void gl_vertices_free(vec3 **positions, vec3 **normals, vec2 **uvs)
{
        if (!positions || !normals || !uvs)
                return;

        memfree((void **)positions);
        memfree((void **)normals);
        memfree((void **)uvs);
}

/**
 * VBOs
 */

#define GL_VBO_EXPAND_COUNT                     (32)

int gl_vbo_init(gl_vbo *vbo)
{
        int ret;

        if (!vbo)
                return -EINVAL;

        ret = seqlist_init(&vbo->indices, sizeof(uint32_t), GL_VBO_EXPAND_COUNT);
        if (ret) {
                goto err;
        }

        ret = seqlist_init(&vbo->vbo_attrs, sizeof(vertex_attr), GL_VBO_EXPAND_COUNT);
        if (ret) {
                goto err;
        }

        return 0;

err:
        // seqlist_deinit() will validate data itself
        seqlist_deinit(&vbo->indices);
        seqlist_deinit(&vbo->vbo_attrs);
        return ret;
}

int gl_vbo_deinit(gl_vbo *vbo)
{
        if (!vbo)
                return -EINVAL;

        seqlist_deinit(&vbo->indices);
        seqlist_deinit(&vbo->vbo_attrs);

        return 0;
}

int vertex_is_indexed(seqlist *vbo_list, vertex_attr *new_attr, uint32_t *idx)
{
        // FIXME: Linear search
        for (uint32_t i = 0; i < vbo_list->count_utilized; ++i) {
                vertex_attr *attr = &((vertex_attr *)vbo_list->data)[i];

                if (vec3_equal(attr->position, new_attr->position) &&
                    vec3_equal(attr->normal, new_attr->normal) &&
                    vec2_equal(attr->uv, new_attr->uv)) {
                        *idx = i;
                        return 1;
                }
        }

        return 0;
}

int gl_vbo_index(gl_vbo *vbo, vertex_attr *vertices, uint32_t vertex_count)
{
        for (uint32_t i = 0; i < vertex_count; ++i) {
                vertex_attr *vertex_pack = &vertices[i];
                uint32_t idx;

                if (vertex_is_indexed(&vbo->vbo_attrs, vertex_pack, &idx)) {
                        seqlist_append(&vbo->indices, &idx);
                } else {
                        uint32_t new_idx = (uint32_t)vbo->vbo_attrs.count_utilized;

                        seqlist_append(&vbo->indices, &new_idx);
                        seqlist_append(&vbo->vbo_attrs, vertex_pack);
                }
        }

        seqlist_shrink(&vbo->indices);
        seqlist_shrink(&vbo->vbo_attrs);

        return 0;
}

void gl_vbo_vertices_copy(gl_vbo *vbo, vec3 *vertex, vec3 *normal, vec2 *uv)
{
        for (size_t i = 0; i < vbo->vbo_attrs.count_utilized; ++i) {
                vertex_attr *attr = &((vertex_attr *)vbo->vbo_attrs.data)[i];

                memcpy(&vertex[i], attr->position, sizeof(vec3));
                memcpy(&normal[i], attr->normal, sizeof(vec3));
                memcpy(&uv[i], attr->uv, sizeof(vec2));
        }
}

int gl_vbo_buffer_create(gl_vbo *vbo, gl_attr *glattr)
{
        vec2 *uvs;
        vec3 *normals;
        vec3 *positions;
        size_t vertex_count;
        int ret;

        if (!vbo || !glattr)
                return -EINVAL;

        vertex_count = vbo->vbo_attrs.count_utilized;

        gl_vertices_alloc(&positions, &normals, &uvs, vertex_count);
        gl_vbo_vertices_copy(vbo, positions, normals, uvs);

        glattr->vertex_count = (GLsizei)vbo->indices.count_utilized;
        glattr->vbo_index = buffer_element_create(vbo->indices.data,
                                                  vbo->indices.element_size *
                                                  vbo->indices.count_utilized);
        ret = glIsBuffer(glattr->vbo_index);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex indexed buffer\n");
                goto free_alloc;
        }

        glattr->vertex = buffer_create(positions, sizeof(vec3) * vertex_count);
        ret = glIsBuffer(glattr->vertex);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex buffer\n");
                goto del_indices;
        }

        glattr->vertex_nrm = buffer_create(normals, sizeof(vec3) * vertex_count);
        ret = glIsBuffer(glattr->vertex_nrm);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex normal buffer\n");
                goto del_vertex;
        }

        glattr->vertex_uv = buffer_create(uvs, sizeof(vec2) * vertex_count);
        ret = glIsBuffer(glattr->vertex_uv);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex uv buffer\n");
                goto del_normal;
        }

free_alloc:
        gl_vertices_free(&positions, &normals, &uvs);

        return ret;

del_indices:
        buffer_delete(&glattr->vbo_index);

del_vertex:
        buffer_delete(&glattr->vertex);

del_normal:
        buffer_delete(&glattr->vertex_nrm);

        goto free_alloc;
}

int gl_vbo_is_empty(gl_vbo *vbo)
{
        if (!vbo)
                return 1;

        if (seqlist_is_empty(&vbo->indices) &&
            seqlist_is_empty(&vbo->vbo_attrs))
                return 1;

        return 0;
}

/**
 * String Rendering
 */

#define TEXTURE_FONT(file)            (RESOURCES_TEXTURE_PATH file SUFFIX_PNG)
#define TEXT_SHADER(file)             (RESOURCES_SHADER_PATH file SUFFIX_GLSL)

typedef struct text_font {
        const char      *texel_file;
        const char      *texel_file_bg;
        int             texel_width;
        int             texel_height;
        int             cell_width;
        int             cell_height;
        int             char_width;
        int             char_height;
        image_png       png;
        image_png       png_bg;
        GLuint          texel;
        GLuint          texel_bg;
        gl_attr         glattr;
} text_font;

static text_font font_ubuntu = {
        .texel_file     = TEXTURE_FONT("font_ubuntu"),
        .texel_file_bg  = TEXTURE_FONT("font_ubuntu_bg"),
        .texel_width    = 512,
        .texel_height   = 512,
        .cell_width     = 32,
        .cell_height    = 32,
        .char_width     = 12,
        .char_height    = 24,
};

static text_font *g_font = &font_ubuntu;

void text_string_prepare(text_font *font, const char *str, int x, int y,
                         float scale, int fb_w, int fb_h,
                         seqlist *vertices, seqlist *uvs)
{
        UNUSED_PARAM(fb_w);

        enum corner {
                UL = V1,
                UR = V2,
                LL = V3,
                LR = V4,
        };

        enum uv_attr {
                U = 0,
                V,
        };

        /**
         *
         * (x1, y2) ----- (x2, y2)    ___
         * |  UL            UR   |     |
         * |                     |     |
         * |                     |     |
         * |                     | char height
         * |                     |     |
         * |  LL            LR   |     |
         * (x1, y1) ----- (x2, y1)    _|_
         *
         * |---- char width -----|
         *
         */

        int column = font->texel_width / font->cell_width;
        int row = font->texel_height / font->cell_height;

        int char_w = font->char_width;
        int char_h = font->char_height;

        float uv_x_div = 1.0f / (float)column;
        float uv_y_div = 1.0f / (float)row;

        float uv_h = (float)char_h / font->cell_height;
        float uv_w = (float)char_w / font->cell_width;

        for (size_t i = 0; i < strlen(str); ++i) {
                vec2 vertex[VERTICES_QUAD];

                float x1 = x + (i * char_w) * scale;
                float y1 = fb_h - (char_h * scale) - y; // Move (0, 0) from frame LL to UL
                float x2 = x1 + char_w * scale;
                float y2 = y1 + char_h * scale;

                vertex[UL][X] = x1; vertex[UL][Y] = y2;
                vertex[UR][X] = x2; vertex[UR][Y] = y2;
                vertex[LL][X] = x1; vertex[LL][Y] = y1;
                vertex[LR][X] = x2; vertex[LR][Y] = y1;

                // Two triangles, clock-wise
                seqlist_append(vertices, vertex[UL]);
                seqlist_append(vertices, vertex[UR]);
                seqlist_append(vertices, vertex[LL]);

                seqlist_append(vertices, vertex[UR]);
                seqlist_append(vertices, vertex[LR]);
                seqlist_append(vertices, vertex[LL]);

                char charcode = str[i];
                float uv_x = (charcode % column) / (float)column;
                float uv_y = (charcode / row) / (float)row;
                vec2 uv[VERTICES_QUAD];

                x1 = uv_x;
                y1 = uv_y + uv_y_div * uv_h;
                x2 = uv_x + uv_x_div * uv_w;
                y2 = uv_y;

                uv[UL][U] = x1; uv[UL][V] = y2;
                uv[UR][U] = x2; uv[UR][V] = y2;
                uv[LL][U] = x1; uv[LL][V] = y1;
                uv[LR][U] = x2; uv[LR][V] = y1;

                seqlist_append(uvs, uv[UL]);
                seqlist_append(uvs, uv[UR]);
                seqlist_append(uvs, uv[LL]);

                seqlist_append(uvs, uv[UR]);
                seqlist_append(uvs, uv[LR]);
                seqlist_append(uvs, uv[LL]);
        }

        seqlist_shrink(vertices);
        seqlist_shrink(uvs);
}

int text_string_draw(const char *str, int x, int y, float scale, int background,
                     int fb_width, int fb_height)
{
        float screen_size[2] = { fb_width, fb_height };
        text_font *font = g_font;
        gl_attr *glattr = &font->glattr;
        seqlist vertices;
        seqlist uvs;

        if (!str)
                return -EINVAL;

        seqlist_init(&vertices, sizeof(vec2), 32);
        seqlist_init(&uvs, sizeof(vec2), 32);

        text_string_prepare(font, str, x, y, scale, fb_width, fb_height, &vertices, &uvs);

        if (background)
                glattr->texel = font->texel_bg;
        else
                glattr->texel = font->texel;

        glattr->vertex = buffer_create(vertices.data,
                                       vertices.element_size * vertices.count_utilized);
        glattr->vertex_uv = buffer_create(uvs.data,
                                          uvs.element_size * uvs.count_utilized);

        glUseProgram(glattr->program);

        glUniform2fv(glattr->uniform_1, 1, &screen_size[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glattr->texel);
        glUniform1i(glattr->sampler, 0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex_uv);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.count_utilized);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        buffer_delete(&glattr->vertex);
        buffer_delete(&glattr->vertex_uv);

        seqlist_deinit(&vertices);
        seqlist_deinit(&uvs);

        return 0;
}

int text_render_init(void)
{
        text_font *font = g_font;
        gl_attr *glattr = &font->glattr;
        int ret;

        ret = image_png32_load(&font->png, font->texel_file);
        if (ret)
                return ret;

        ret = image_png32_load(&font->png_bg, font->texel_file_bg);
        if (ret)
                goto free_png;

        gl_attr_init(glattr);

        font->texel = texture_png_create(&font->png, FILTER_NEAREST, 0);
        ret = glIsTexture(font->texel);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create font texture\n");
                goto free_png_bg;
        }

        font->texel_bg = texture_png_create(&font->png_bg, FILTER_NEAREST, 0);
        ret = glIsTexture(font->texel_bg);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create font texture\n");
                goto free_texel;
        }

        glattr->program = program_create(TEXT_SHADER("text_vertex"),
                                           TEXT_SHADER("text_fragment"));
        ret = glIsProgram(glattr->program);
        if (ret == GL_FALSE) {
                pr_err_func("failed to load font shaders\n");
                goto free_texel_bg;
        }

        glattr->sampler = glGetUniformLocation(font->glattr.program, "sampler");
        glattr->uniform_1 = glGetUniformLocation(font->glattr.program, "screen_size");

        return 0;

free_texel_bg:
        texture_delete(&font->texel_bg);

free_texel:
        texture_delete(&font->texel);

free_png_bg:
        image_png_free(&font->png_bg);

free_png:
        image_png_free(&font->png);

        return ret;
}

int text_render_deinit(void)
{
        text_font *font = g_font;
        gl_attr *glattr = &font->glattr;

        image_png_free(&font->png);
        image_png_free(&font->png_bg);

        if (glIsTexture(font->texel) != GL_FALSE) {
                texture_delete(&font->texel);
        }

        if (glIsTexture(font->texel_bg) != GL_FALSE) {
                texture_delete(&font->texel_bg);
        }

        if (glIsProgram(font->glattr.program) != GL_FALSE) {
                program_delete(&font->glattr.program);
        }

        gl_attr_buffer_delete(glattr);

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
                fps->fps = fps->frame_count;
                fps->frame_time_ms = ms_1000 / (double)fps->frame_count;

                fps->frame_count = 0;
                fps->frame_time_last = glfwGetTime();
        }
}

void fps_meter_draw(fps_meter *fps, int fb_width, int fb_height)
{
        char buf[256];

        sprintf_s(buf, sizeof(buf), " %d fps (%.4lf ms) ",
                  fps->fps, fps->frame_time_ms);

        text_string_draw(buf, 0, 0, 1, 1, fb_width, fb_height);
}
