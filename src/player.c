#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "debug.h"
#include "block.h"
#include "model.h"
#include "utils.h"
#include "glutils.h"
#include "chunks.h"
#include "player.h"

#define ANGEL_VERTICAL_MIN                      (M_PI / (-2))
#define ANGEL_VERTICAL_MAX                      (M_PI / (2))

#define ANGEL_HORIZONTAL_MIN                    (0.0)
#define ANGEL_HORIZONTAL_MAX                    (M_PI * 2)

#define VERTICES_COLLISION_TEST                 (12)

static player default_player = {
        .origin_gl = { 0.0f, 1.95f / 2.0f , 5.0f },

        .size = {
                .height = 1.95f * BLOCK_EDGE_LEN_GLUNIT,
                .length = 0.5f * BLOCK_EDGE_LEN_GLUNIT,
                .width  = 0.5f * BLOCK_EDGE_LEN_GLUNIT,
        },

        .state = STANDING,
        .speed = {
                .fly = 10.0,
                .fly_sprint = 15.0,

                .walk = 3.0,
                .sprint = 6.0,
                .sneak = 1.0,

                .jump = 20.0,
                .jump_air = 2.0,

                .view = 0.001,
        },

        .cam = {
                .fov = FOV_NORMAL,
                .fov_delta = +5.0f,

                .angel_vertical = 0.0,
                .angel_horizontal = M_PI,

                .clamp_far = 256.0f,
                .clamp_near = 0.1f,
        },
        .cam_offset = { 0.0f, +0.66f, 0.0f },
};

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

void camera_position_update(camera *cam, player *p)
{
        glm_vec_add(p->origin_gl, p->cam_offset, cam->position);
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


static inline int __point_on_cube_edge(const vec3 point_gl, int l)
{
        if (float_equal(fmodf(point_gl[X], l), 0.0f, FLT_EPSILON) ||
            float_equal(fmodf(point_gl[Y], l), 0.0f, FLT_EPSILON) ||
            float_equal(fmodf(point_gl[Z], l), 0.0f, FLT_EPSILON))
        {
                return 1;
        }

        return 0;
}

static inline int __point_to_cube(float x, int l)
{
        int ret;

        // Fix edge case
        if (fabsf(x) < l) {
                if (x > 0)
                        return 0;
                else if (x < 0)
                        return -1;
        }

        ret = (int)(x / (float)l);

        // On world negative axi,
        // block start counting with (-1)
        if (x < 0)
                ret -= 1;

        return ret;
}

/**
 * point_to_cube() - point in world space to find in which cube (local origin)
 *
 * @param point_gl: point in world space
 * @param edge_len: cube edge
 * @param origin_l: cube local origin
 * @return 0 on cube edge.
 */
int point_to_cube(const vec3 point_gl, int edge_len, ivec3 origin_l)
{
        if (__point_on_cube_edge(point_gl, edge_len))
                return 0;

        origin_l[X] = __point_to_cube(point_gl[X], edge_len);
        origin_l[Y] = __point_to_cube(point_gl[Y], edge_len);
        origin_l[Z] = __point_to_cube(point_gl[Z], edge_len);

        return 1;
}

/**
 * collision_test_block_point() - test a point is in a block or not
 *
 * @param point_gl: point in world space
 * @return 1 on collision detected
 */
int collision_test_block_point(world *w, const vec3 point_gl)
{
        ivec3 origin_b = { 0 };

        if (!point_to_cube(point_gl, BLOCK_EDGE_LEN_GLUNIT, origin_b))
                return 0;

        if (world_get_block(w, origin_b) != NULL)
                return 1;

        return 0;
}

void player_collision_test_vertices(vec3 *vertices, const vec3 origin,
                                    dimension size)
{
        float delta_x = size.width / 2;
        float delta_y = size.height / 2;
        float delta_z = size.length / 2;

        vertices[V1][X] = origin[X] - delta_x;
        vertices[V1][Y] = origin[Y] - delta_y;
        vertices[V1][Z] = origin[Z] - delta_z;

        vertices[V2][X] = origin[X] + delta_x;
        vertices[V2][Y] = origin[Y] - delta_y;
        vertices[V2][Z] = origin[Z] - delta_z;

        vertices[V3][X] = origin[X] + delta_x;
        vertices[V3][Y] = origin[Y] - delta_y;
        vertices[V3][Z] = origin[Z] + delta_z;

        vertices[V4][X] = origin[X] - delta_x;
        vertices[V4][Y] = origin[Y] - delta_y;
        vertices[V4][Z] = origin[Z] + delta_z;

        vertices[V5][X] = origin[X] - delta_x;
        vertices[V5][Y] = origin[Y];
        vertices[V5][Z] = origin[Z] - delta_z;

        vertices[V6][X] = origin[X] + delta_x;
        vertices[V6][Y] = origin[Y];
        vertices[V6][Z] = origin[Z] - delta_z;

        vertices[V7][X] = origin[X] + delta_x;
        vertices[V7][Y] = origin[Y];
        vertices[V7][Z] = origin[Z] + delta_z;

        vertices[V8][X] = origin[X] - delta_x;
        vertices[V8][Y] = origin[Y];
        vertices[V8][Z] = origin[Z] + delta_z;

        vertices[V9][X] = origin[X] - delta_x;
        vertices[V9][Y] = origin[Y];
        vertices[V9][Z] = origin[Z] - delta_z;

        vertices[V10][X] = origin[X] + delta_x;
        vertices[V10][Y] = origin[Y] + delta_y;
        vertices[V10][Z] = origin[Z] - delta_z;

        vertices[V11][X] = origin[X] + delta_x;
        vertices[V11][Y] = origin[Y] + delta_y;
        vertices[V11][Z] = origin[Z] + delta_z;

        vertices[V12][X] = origin[X] - delta_x;
        vertices[V12][Y] = origin[Y] + delta_y;
        vertices[V12][Z] = origin[Z] + delta_z;
}

int player_collision_test(player *p, world *w, vec3 origin_t)
{
        vec3 test_vertices[VERTICES_COLLISION_TEST];

        player_collision_test_vertices(test_vertices, origin_t, p->size);

        for (int i = 0; i < VERTICES_COLLISION_TEST; ++i) {
                if (collision_test_block_point(w, test_vertices[i]))
                        return 1;
        }

        return 0;
}

void player_move_on_ground(player *p, world *w, GLFWwindow *window)
{
        camera *cam = &p->cam;
        float speed = p->speed.walk;
        vec3 abs_up = { 0.0f, 1.0f, 0.0f };
        vec3 abs_front = { 0.0f };
        vec3 origin_t = { 0.0f };
        vec3 t = { 0.0f };

        // TODO: State change
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                speed = p->speed.sprint;
        } else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) ==  GLFW_PRESS) {
                speed = p->speed.sneak;
        }

        // Vectors for forward/backward
        glm_cross(cam->vector_right, abs_up, abs_front);
        glm_vec_scale(abs_front, cam->time_delta, t);
        glm_vec_scale(t, speed, t);
        glm_vec_inv(t);

        // TODO: Move range check
        // TODO: Clamp

        // Forward
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[X] += t[X];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[X] = origin_t[X];

                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[Z] += t[Z];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[Z] = origin_t[Z];
        }

        // Backward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[X] -= t[X];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[X] = origin_t[X];

                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[Z] -= t[Z];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[Z] = origin_t[Z];
        }

        // Vector for left/right
        glm_vec_scale(cam->vector_right, cam->time_delta, t);
        glm_vec_scale(t, speed, t);

        // Left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[X] -= t[X];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[X] = origin_t[X];

                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[Z] -= t[Z];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[Z] = origin_t[Z];
        }

        // Right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[X] += t[X];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[X] = origin_t[X];

                glm_vec_copy(p->origin_gl, origin_t);
                origin_t[Z] += t[Z];
                if (!player_collision_test(p, w, origin_t))
                        p->origin_gl[Z] = origin_t[Z];
        }
}

void player_movement_handle(player *p, world *w, GLFWwindow *window)
{
        player_movement state = p->state;

        switch (state) {
                case FLYING:
                case FALLING:
                        break;

                default:
                case STANDING:
                case WALKING:
                case RUNNING:
                case SNEAKING:
                        player_move_on_ground(p, w, window);
                        break;
        }
}

void player_inputs_handle(player *p, world *w, GLFWwindow *window)
{
        camera *cam = &p->cam;
        player_speed *speed = &p->speed;

        camera_deltatime_compute(cam);

        camera_vectors_compute(window, cam, speed->view);

        player_movement_handle(p, w, window);

        camera_position_update(cam, p);

        camera_perspective_compute(cam, 0);
}

int player_hint(player *p, player *hint)
{
        if (!p)
                return -EINVAL;

        memcpy(p, hint, sizeof(player));

        return 0;
}

int player_default(player *p)
{
        if (!p)
                return -EINVAL;

        player_hint(p, &default_player);

        return 0;
}

int player_init(player *p)
{
        if (!p)
                return -EINVAL;

        p->cam.time_last = glfwGetTime();

        return 0;
}

int player_deinit(player *p)
{
        if (!p)
                return -EINVAL;

        return 0;
}
