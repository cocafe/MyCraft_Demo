#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <errno.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "debug.h"

// Testy Triangle
static const GLfloat g_vertex_buffer_data[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
         1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f
};

static const GLfloat g_color_buffer_data[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
};

char *file_read(const char *filepath)
{
        errno_t err;
        FILE *fp;
        char *buf;
        char ch;
        size_t fsize;
        size_t i;

        err = fopen_s(&fp, filepath, "r");
        if (!fp) {
                pr_err_func("failed to open file %s (err: %d)\n", filepath, err);
                return NULL;
        }

        fsize = 0;
        while (1) {
                ch = (char)fgetc(fp);

                if (ferror(fp)) {
                        pr_err_func("failed to read file %s\n", filepath);
                        return NULL;
                }

                if (feof(fp)) {
                        break;
                }

                fsize++;
        }

        buf = calloc(1, sizeof(char) * (fsize + 2));
        if (!buf) {
                pr_err_alloc();
                return NULL;
        }

        memset(buf, '\0', sizeof(char) * (fsize + 2));

        rewind(fp);
        i = 0;
        while (1) {
                ch = (char)fgetc(fp);

                if (ferror(fp)) {
                        pr_err_func("failed to read file %s\n", filepath);
                        return NULL;
                }

                if (feof(fp)) {
                        break;
                }

                buf[i] = ch;
                i++;
        }

        return buf;
}

char *buf_alloc(size_t len)
{
        if (!len)
                return NULL;

        char *buf = calloc(1, sizeof(char) * len);
        if (!buf)
                return NULL;

        memset(buf, '\0', sizeof(char) * len);

        return buf;
}

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

int main()
{
        GLFWwindow *window;

        // GLFW init
        if (!glfwInit()) {
                return EXIT_FAILURE;
        }

        // GLFW window option: MSAA
        glfwWindowHint(GLFW_SAMPLES, 4);

        // GLFW OpenGL context options
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // GLFW create window
        window = glfwCreateWindow(1280, 720, "MyCraft Demo", NULL, NULL);
        if (!window) {
                glfwTerminate();
                return EXIT_FAILURE;
        }

        // GLFW make OpenGL context
        glfwMakeContextCurrent(window);

        // GLEW init
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
                glfwTerminate();
                return EXIT_FAILURE;
        }

        // GLFW window input options
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, 1280 / 2, 720 / 2);

        // GLFW flush unhandled events
        glfwPollEvents();

        // GLFW window vsync settings:
        //      -1: auto
        //      1: on
        //      0: off
        glfwSwapInterval(-1);

        // OpenGL background color: RGBA
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // OpenGL hardware Z-Buffer
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it's closer to camera then the former one
        glDepthFunc(GL_LESS);

        // VAO
        GLuint vertex_array_id;
        glGenVertexArrays(1, &vertex_array_id);
        glBindVertexArray(vertex_array_id);

        // Mesh vertex buffer
        GLuint vertex_buffer;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                     g_vertex_buffer_data, GL_STATIC_DRAW);

        // Mesh color buffer
        GLuint color_buffer;
        glGenBuffers(1, &color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data),
                     g_color_buffer_data, GL_STATIC_DRAW);

        // Compile GLSL shaders
        GLuint shader_program_id;
        if (shaders_load(&shader_program_id,
                     "resource/shaders/dummy_vertex_shader.glsl",
                     "resource/shaders/dummy_fragment_shader.glsl") != GL_TRUE)
                goto free_buffers;

        // Variable will be pass to GLSL
        GLint MVP_uniform = glGetUniformLocation(shader_program_id, "MVP");

        mat4 mat_trans = {0};
        glm_mat4_identity(mat_trans);

        vec3 vec_trans = { 0.0f, 0.0f, -3.0f };
        glm_translate(mat_trans, vec_trans);
        glm_mat4_print(mat_trans, stdout);
        fflush(stdout);

        mat4 mat_model = {0};
        glm_mat4_identity(mat_model);
        glm_mat4_print(mat_model, stdout);
        fflush(stdout);

        mat4 mat_camera = {0};
        glm_lookat((vec3){ 4, 3, 3 }, (vec3){ 0, 0, 0 }, (vec3){ 0, 1, 0 }, mat_camera);
        glm_mat4_print(mat_camera, stdout);
        fflush(stdout);

        mat4 mat_project = {0};
        glm_perspective(96.0f, 16.0f / 9.0f, 0.1f, 100.0f, mat_project);
        glm_mat4_print(mat_project, stdout);
        fflush(stdout);

        mat4 mat_mvp = {0};
        glm_mat4_mulN((mat4 *[]){&mat_project, &mat_camera, &mat_model, &mat_trans}, 4, mat_mvp);

        glm_mat4_print(mat_mvp, stdout);
        fflush(stdout);

        do {
                // Clear the screen
                glClear(GL_COLOR_BUFFER_BIT |
                        GL_DEPTH_BUFFER_BIT |
                        GL_STENCIL_BUFFER_BIT);

                // Use our shader program during rendering
                glUseProgram(shader_program_id);

                // Send MVP matrix to GLSL
                glUniformMatrix4fv(MVP_uniform, 1, GL_FALSE, &mat_mvp[0][0]);

                // Attribute 0 passes to GLSL: vertices
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

                // Attribute 1 passes to GLSL: color
                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

                glDrawArrays(GL_TRIANGLES, 0, (6 * 2) * 3);

                glDisableVertexAttribArray(0);

                glfwSwapBuffers(window);
                glfwPollEvents();
        } while((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) &&
                (glfwWindowShouldClose(window) == false));

        glDeleteProgram(shader_program_id);
free_buffers:
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteVertexArrays(1, &vertex_array_id);

free_glfw:
        glfwTerminate();

        return 0;
}