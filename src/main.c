#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "debug.h"
#include "block.h"
#include "utils.h"
#include "model.h"
#include "texel.h"
#include "chunks.h"
#include "player.h"
#include "world.h"
#include "thread.h"
#include "mycraft.h"

static mc_config def_config = {
        .vsync                  = VSYNC_ADAPTIVE,
        .window_width           = PROGRAM_WINDOW_WIDTH,
        .window_height          = PROGRAM_WINDOW_HEIGHT,
        .fullscreen             = false,                // TODO: Fullscreen switch
        .save_path              = "./",
        .fov                    = FOV_NORMAL,
        .render_dist            = 8,
        .crosshair_show         = true,
        .texture_filter_level   = FILTER_NEAREST,
        .texture_mipmap_level   = 4,
        .debug_level            = PRINT_INFO_BIT | PRINT_ERROR_BIT | PRINT_DEBUG_BIT,
        .opengl_msaa            = 4,
};

static mc_program def_program;
mc_program *g_program = &def_program;

int mc_config_init()
{
        // TODO: Read configs from disk
        return 0;
}

int mc_program_init(mc_program *program, mc_config *config)
{
        if (!program || !config)
                return -EINVAL;

        memset(program, 0x00, sizeof(mc_program));

        memcpy(&program->config, config, sizeof(mc_config));

        program->window_width = config->window_width;
        program->window_height = config->window_height;

        return 0;
}

void mc_program_key_callback(mc_program *program, int key, int action)
{
        switch (key) {
                case GLFW_KEY_ESCAPE:
                        if (action == GLFW_PRESS)
                                program->state = PROGRAM_EXIT;

                        break;

                case GLFW_KEY_KP_3:
                        if (action == GLFW_PRESS) {
                                world_update_trigger(&program->mc_world);
                        }
                        break;

                case GLFW_KEY_G:
                        if (action == GLFW_PRESS) {
                                int t = program->mc_player.attr.fly_noclip;
                                program->mc_player.attr.fly_noclip = !t;
                        }
                        break;

                default:
                        break;
        }
}

int glfw_init(void)
{
        if (!glfwInit()) {
                pr_err_func("failed to init GLFW\n");
                return -EFAULT;
        }

        return 0;
}

void glfw_deinit(void)
{
        glfwTerminate();
}

void glfw_window_close_callback(GLFWwindow *window)
{
        UNUSED_PARAM(window);

        g_program->state = PROGRAM_EXIT;
}

void glfw_window_focus_callback(GLFWwindow *window, int focus)
{
        UNUSED_PARAM(window);

        if (focus == GLFW_TRUE) {
                g_program->focus = 1;
                g_program->state = PROGRAM_RUNNING;
        } else {
                g_program->focus = 0;
                g_program->state = PROGRAM_PAUSE;
        }
}

void glfw_key_input_callback(GLFWwindow *window, int key,
                             int scancode, int action, int mods)
{
        UNUSED_PARAM(window);
        UNUSED_PARAM(scancode);
        UNUSED_PARAM(mods);

        if (g_program->state != PROGRAM_RUNNING)
                return;

        mc_program_key_callback(g_program, key, action);
        player_key_callback(&g_program->mc_player, key, action);
}

void glfw_mouse_input_callback(GLFWwindow *window, int button,
                               int action, int mods)
{
        UNUSED_PARAM(window);
        UNUSED_PARAM(mods);

        player_mouse_callback(&g_program->mc_player, &g_program->mc_world,
                              button, action);
}

void glfw_scroll_input_callback(GLFWwindow *window, double xoffset, double yoffset)
{
        UNUSED_PARAM(window);

        player_scroll_callback(&g_program->mc_player, xoffset, yoffset);
}

void glfw_window_resize_callback(GLFWwindow *window, int width, int height)
{
        UNUSED_PARAM(window);

        glViewport(0, 0, width, height);

        g_program->window_width = width;
        g_program->window_height = height;
}

int glfw_window_init(GLFWwindow **window, int width, int height, int msaa, int vsync)
{
        glfwWindowHint(GLFW_SAMPLES, msaa);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_API_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_API_MINOR);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        *window = glfwCreateWindow(width, height, PROGRAM_WINDOW_TITLE, NULL, NULL);
        if (!*window) {
                pr_err_func("failed to create render window\n");
                return -EFAULT;
        }

        // TODO: Set windows to monitor center

        glfwMakeContextCurrent(*window);

        glfwSwapInterval(vsync);

        return 0;
}

void glfw_input_init(GLFWwindow *window)
{
        int width;
        int height;

        glfwGetWindowSize(window, &width, &height);

        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, width / 2.0, height / 2.0);
}

void glfw_callback_init(GLFWwindow *window)
{
        glfwSetKeyCallback(window, glfw_key_input_callback);
        glfwSetScrollCallback(window, glfw_scroll_input_callback);
        glfwSetMouseButtonCallback(window, glfw_mouse_input_callback);

        glfwSetWindowFocusCallback(window, glfw_window_focus_callback);
        glfwSetWindowCloseCallback(window, glfw_window_close_callback);
        glfwSetWindowSizeCallback(window, glfw_window_resize_callback);
}

int glew_init(void)
{
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
                pr_err_func("failed to init GLEW\n");
                return -EFAULT;
        }

        return 0;
}

void opengl_extension(void)
{
        if (GLEW_ARB_debug_output) {
                pr_debug_func("Enable GL_ARB_DEBUG_OUTPUT extension\n");
                glDebugMessageCallbackARB(&opengl_debug_output_callback, NULL);
//                glEnable(GL_DEBUG_OUTPUT);
        }
}

void opengl_set(void)
{
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClear(GL_STENCIL_BUFFER_BIT);

        glFrontFace(GL_CW);

//        glEnable(GL_CULL_FACE);

        glDisable(GL_LINE_SMOOTH);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(const int argc, const char *argv[])
{
        mc_program *program = &def_program;
        player *mc_player = &program->mc_player;
        world *mc_world = &program->mc_world;
        int ret;

        // Cmdline process

        if (argc > 1) {
                pr_debug_func("cmdline: ");
                for (int i = 1; i < argc; ++i) {
                        pr_debug("%s ", argv[i]);
                }
                pr_debug("\n");
        }

        // Program config init

        mc_program_init(program, &def_config);

        // GLFW init

        ret = glfw_init();
        if (ret) {
                goto out;
        }

        ret = glfw_window_init(&program->window,
                               program->window_width,
                               program->window_height,
                               program->config.opengl_msaa,
                               program->config.vsync);
        if (ret) {
                goto out_glfw;
        }

        glfw_input_init(program->window);
        glfw_callback_init(program->window);

        program->focus = 1;
        program->state = PROGRAM_RUNNING;

        // TODO: GLFW set window to monitor center

        // GLEW init

        ret = glew_init();
        if (ret) {
                goto out_glfw;
        }

        // OpenGL init

        opengl_set();
        opengl_extension();

        program->VAO = vertex_array_create();
        if (glIsVertexArray(program->VAO) == GL_FALSE) {
                pr_err_func("failed to init OpenGL VAO\n");
                goto out_glfw;
        }

        // Resources init

        ret = texel_pack_init();
        if (ret)
                goto out_vao;

        ret = block_shader_init();
        if (ret) {
                goto out_texel_pack;
        }

        ret = block_attr_init();
        if (ret) {
                goto out_block_shader;
        }

        fps_meter_init(&program->fps);

        player_default(mc_player);
        player_init(mc_player);

        line_render_init();
        text_render_init();
        thread_helper_init();
        crosshair_textured_init();

        world_init(mc_world);

        super_flat_generate(mc_world, SUPER_FLAT_GRASS, 128, 128);
        world_update_chunks(mc_world, 0);
        world_worker_create(mc_world);

        player_position_set(mc_player, (vec3){ 32, 10, 32 });

        do {
                // Global clear call for next frame
                glClear(GL_COLOR_BUFFER_BIT);
                glClear(GL_DEPTH_BUFFER_BIT);

                player_inputs_process(mc_player, mc_world, program->window);

                world_draw_chunks(mc_world, mc_player->cam.position, mc_player->cam.mat_transform);

                if (mc_player->hittest.hit) {
                        block_wireframe_draw(mc_player->hittest.origin_b,
                                             (color_rgba)RGBA_COLOR(RGB_GLSL_BLACK, 0.6),
                                             0, mc_player->cam.mat_transform);
                }

                glClear(GL_DEPTH_BUFFER_BIT);
                fps_meter_count(&program->fps);
                fps_meter_measure(&program->fps);
                fps_meter_draw(&program->fps,
                               program->window_width,
                               program->window_height);

                player_info_draw(mc_player,
                                 program->window_width,
                                 program->window_height);

                crosshair_textured_draw(1.0f, program->window_width,
                                        program->window_height);

                glfwSwapBuffers(program->window);
                glfwPollEvents();
        } while(program->state != PROGRAM_EXIT);

        crosshair_textured_deinit();
        text_render_deinit();
        line_render_deinit();
        thread_helper_deinit();

        world_deinit(mc_world);
        player_deinit(mc_player);

        block_attr_deinit();

out_block_shader:
        block_shader_deinit();

out_texel_pack:
        texel_pack_deinit();

out_vao:
        vertex_array_delete(&program->VAO);

out_glfw:
        glfw_deinit();

out:
        // Wait for threads
//        pthread_exit(NULL);

        return ret;
}