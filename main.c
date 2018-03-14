#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "cglm_helper.h"

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
        glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

        do {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                glfwSwapBuffers(window);
                glfwPollEvents();
        } while((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) &&
                (glfwWindowShouldClose(window) == false));

        glfwTerminate();

        return EXIT_SUCCESS;
}