#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "debug.h"
#include "block.h"
#include "model.h"
#include "utils.h"
#include "glutils.h"
#include "player.h"

#define ANGEL_VERTICAL_MIN                      (M_PI / (-2))
#define ANGEL_VERTICAL_MAX                      (M_PI / (2))

#define ANGEL_HORIZONTAL_MIN                    (0.0)
#define ANGEL_HORIZONTAL_MAX                    (M_PI * 2)

int camera_init(camera *cam, camera *hint)
{
        if (!cam || !hint)
                return -EINVAL;

        memcpy(cam, hint, sizeof(camera));

        // TODO: Adjust camera positon upon player position

        return 0;
}

void camera_deltatime_compute(camera *cam)
{
        cam->time_curr = glfwGetTime();
        cam->time_delta = (float)(cam->time_curr - cam->time_last);
        cam->time_last = cam->time_curr;
}

void camera_vectors_compute(GLFWwindow *window, camera *cam, double speed)
{
        double x, y;
        double set_x, set_y;

        // TODO: GLFW Callback
        glfwGetWindowSize(window, &cam->view_width, &cam->view_height);

        set_x = (double)cam->view_width / 2;
        set_y = (double)cam->view_height / 2;

        // Delta of cursor movement related to screen center
        glfwGetCursorPos(window, &x, &y);
        glfwSetCursorPos(window, set_x, set_y);

//        if (!cursor_is_changed(set_x, x) && !cursor_is_changed(set_y, y))
//                return;

        cam->angel_horizontal += speed * (set_x - x);
        cam->angel_horizontal = cyclelf(cam->angel_horizontal,
                                        ANGEL_HORIZONTAL_MIN,
                                        ANGEL_HORIZONTAL_MAX);


        cam->angel_vertical += speed * (set_y - y);
        cam->angel_vertical = clamplf(cam->angel_vertical,
                                      ANGEL_VERTICAL_MIN,
                                      ANGEL_VERTICAL_MAX);

        // Front vector:
        // Spherical coordinates to Cartesian coordinates conversion
        cam->vector_front[X] = (float)(cos(cam->angel_vertical) * sin(cam->angel_horizontal));
        cam->vector_front[Y] = (float)(sin(cam->angel_vertical));
        cam->vector_front[Z] = (float)(cos(cam->angel_vertical) * cos(cam->angel_horizontal));

        // Right vector:
        // Relative to camera space
        cam->vector_right[X] = (float)(sin(cam->angel_horizontal - M_PI / 2.0));
        cam->vector_right[Y] = 0.0f;
        cam->vector_right[Z] = (float)(cos(cam->angel_horizontal - M_PI / 2.0));

        // Up vector:
        // Perpendicular to front and right vector, cross production to compute
        glm_cross(cam->vector_right, cam->vector_front, cam->vector_up);
}

// TODO: Handle custom keys...? --> GLFW_KEY_CALLBACK (need thread safe?)
void camera_position_compute(GLFWwindow *window, camera *cam, float speed)
{
        vec3 t = {0, 0, 0};

        // TODO: Movement call to a function to handle movement not introduce by inputs

        // Forward
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                // position(x, y, z) += front(x, y, z) * delta_time * speed;
                glm_vec_scale(cam->vector_front, cam->time_delta, t);
                glm_vec_scale(t, speed, t);
                glm_vec_add(cam->position, t, cam->position);
        }

        // Backward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                // position(x, y, z) -= front(x, y, z) * delta_time * speed;
                glm_vec_scale(cam->vector_front, cam->time_delta, t);
                glm_vec_scale(t, speed, t);
                glm_vec_sub(cam->position, t, cam->position);
        }

        // Left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                // position(x, y, z) -= right(x, y, z) * delta_time * speed;
                glm_vec_scale(cam->vector_right, cam->time_delta, t);
                glm_vec_scale(t, speed, t);
                glm_vec_sub(cam->position, t, cam->position);
        }

        // Right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                // position(x, y, z) += right(x, y, z) * delta_time * speed;
                glm_vec_scale(cam->vector_right, cam->time_delta, t);
                glm_vec_scale(t, speed, t);
                glm_vec_add(cam->position, t, cam->position);
        }

        // TODO: character position update
}

// TODO: dynmaic fov during sprint, flying
void camera_perspective_compute(camera *cam, int32_t dynamic_fov)
{
        vec3 look_at = { 0, 0, 0 };

        glm_perspective(dynamic_fov ? cam->fov + cam->fov_delta : cam->fov,
                        (float)cam->view_width / (float)cam->view_height,
                        cam->clamp_near, cam->clamp_far, cam->mat_persp);

        glm_vec_add(cam->position, cam->vector_front, look_at);
        glm_lookat(cam->position, look_at, cam->vector_up, cam->mat_camera);

        glm_mat4_mulN((mat4 *[]){&cam->mat_persp, &cam->mat_camera},
                      2, cam->mat_transform);
}

void camera_inputs_handle(GLFWwindow *window, camera *cam,
                          double speed_move, double speed_view)
{
        // TODO: Check for game pause -> release cursor

        camera_deltatime_compute(cam);
        camera_position_compute(window, cam, (float)speed_move);
        camera_vectors_compute(window, cam, speed_view);
        camera_perspective_compute(cam, 0);
}