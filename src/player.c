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
#include "world.h"
#include "player.h"

#define ANGEL_VERTICAL_MIN                      (M_PI / (-2))
#define ANGEL_VERTICAL_MAX                      (M_PI / (2))

#define ANGEL_HORIZONTAL_MIN                    (0.0)
#define ANGEL_HORIZONTAL_MAX                    (M_PI * 2)

#define VERTICES_COLLISION_TEST                 (12)

#define WORLD_GRAVITY                           (20.0f)

#define JUMP_HEIGHT                             (1.25f * BLOCK_EDGE_LEN_GLUNIT)

static player default_player = {
        .origin_gl = { 0.0f, PLAYER_HEIGHT / 2.0f , 0.0f },

        .size = {
                .height = PLAYER_HEIGHT,
                .length = PLAYER_LENGTH,
                .width  = PLAYER_WIDTH,
        },

        .state = FLYING,

        .attr = {
                .fly = 10.0f,
                .fly_noclip = 1,

                .air = 4.0f,
                .walk = 5.0f,

                .jump = 0.0f,
                .jump_height = JUMP_HEIGHT,

                .mod_sprint = 1.40f,
                .mod_sneak = 0.5f,

                .view = 0.001f,

                .raycast_radius = 5 * BLOCK_EDGE_LEN_GLUNIT,
        },

        .cam = {
                .fov = FOV_NORMAL,
                .fov_zoom = FOV_ZOOMING,
                .zooming = 0,

                .angel_vertical = 0.0,
                .angel_horizontal = M_PI,

                .clamp_far = 256.0f,
                .clamp_near = 0.1f,
        },

        .cam_offset = { 0.0f, +0.66f, 0.0f },
};

static timestamp player_ts;

static block_attr_idx player_items[] = {
        BLOCK_GRASS,
        BLOCK_DIRT,
        BLOCK_STONE,
        BLOCK_TNT,
        BLOCK_DEBUG,
};

static inline block_attr_idx player_item_current_get(player *p)
{
        return p->item.items[p->item.current];
}

void player_item_scroll(player *p, double offset_y)
{
        player_item *item = &p->item;
        double threshold = 1.0f;

        if (offset_y >= threshold) {
                item->current = (item->current + 1) % item->count;
        } else if (offset_y <= (threshold * -1)) {
                item->current--;

                if (item->current < 0)
                        item->current = item->count - 1;
        }
}

int player_item_draw(player *p)
{
        UNUSED_PARAM(p);

        return 0;
}

int player_item_init(player *p)
{
        player_item *item = &p->item;

        item->count = ARRAY_SIZE(player_items);
        if (item->count <= 0)
                return -EINVAL;

        item->current = 0;
        item->items = player_items;

        return 0;
}

typedef struct hit_block {
        block           *b;
        block_face      *f;
} hit_block;

/**
 * line_plane_is_intersected() - check ray is whether intersected with plane
 *
 * @param contact: contact point will be returned
 * @param ray: ray direction
 * @param ray_origin: ray origin point
 * @param normal: normal of plane
 * @param coord: point on plane
 * @return return 1 and contact point on intersection, 0 on false
 */
int line_plane_is_intersected(vec3 contact, vec3 ray, vec3 ray_origin, vec3 normal, vec3 coord)
{
        vec3 r = { 0 };

        float d = glm_dot(normal, coord);

        // Check for parallel case
        if (glm_dot(normal, ray) == 0) {
                return 0;
        }

        float x = (d - glm_dot(normal, ray_origin)) / glm_dot(normal, ray);

        // contact point = ray origin + normalized(ray) * x
        glm_vec_copy(ray, r);
        glm_normalize(r);
        glm_vec_scale(r, x, r);
        glm_vec_add(ray_origin, r, contact);

        return 1;
}

/**
 * check_in_range() - check value in range
 *
 * @param t: value to test
 * @param a: range a
 * @param b: range b
 * @return 1 on t in range [min(a, b), max(a, b)]
 */
static inline int check_in_range(float t, float a, float b)
{
        float min, max;
        float epsilon = 0.0001f;

        if (a < b) {
                min = a; max = b;
        } else { // b >= a
                min = b; max = a;
        }

        // XXX: imprecise in little/greater comparision
        if (float_equal(t, a, epsilon) || float_equal(t, b, epsilon))
                return 1;

        if (t < min)
                return 0;
        else if (t > max)
                return 0;

        return 1;
}

static inline int vec3_in_range(vec3 t, vec3 a, vec3 b)
{
        if (!check_in_range(t[X], a[X], b[X]) ||
            !check_in_range(t[Y], a[Y], b[Y]) ||
            !check_in_range(t[Z], a[Z], b[Z]))
                return 0;
        else
                return 1;
}

/**
 * point_is_on_block_face() - check contact point is on block face
 *
 * this method is very conditional, it requires block is axis-aligned.
 *
 * @param face: block face to detect
 * @param point: contact point
 * @return 1 on true, 0 on false
 */
static inline int point_is_on_block_face(block_face *face, vec3 point)
{
        vec3 v1, v2;

        glm_vec_copy(face->vertices[CUBE_FACE_UL].position, v1);
        glm_vec_copy(face->vertices[CUBE_FACE_LR].position, v2);

        if (!vec3_in_range(point, v1, v2))
                return 0;

        return 1;
}

static inline int block_in_distance(ivec3 o1, ivec3 o2, int distance)
{
        float ret = ivec3_distance(o1, o2);

        if (ret > (float)distance)
                return 0;
        else
                return 1;
}

/**
 * player_hittest_block_face() - check front ray hit a block face
 *
 * @param p: pointer to player
 * @param b: pointer to block
 * @return pointer to hit block face, NULL on failure
 */
block_face *player_hittest_block_face(player *p, block *b)
{
        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *f = &b->model.faces[i];
                camera *cam = &p->cam;
                vec3 ray = { 0 };
                vec3 contact = { 0 };

                glm_vec_add(p->cam.position, p->cam.vector_front, ray);

                if (!f->visible)
                        continue;

                if (!f->vertices)
                        continue;

                if (face_is_back_face(f->vertices[V1].position,
                                      f->normal,
                                      cam->position))
                        continue;

                if (!line_plane_is_intersected(contact,
                                               cam->vector_front,
                                               cam->position,
                                               f->normal,
                                               f->vertices[V1].position))
                        continue;

                if (!point_is_on_block_face(f, contact))
                        continue;

                return f;
        }

        return NULL;
}

hit_block *hit_block_nearest_get(linklist *list, ivec3 origin_p)
{
        hit_block *ret = NULL;
        linklist_node *pos;
        float dist = 0;

        linklist_for_each_node(pos, list->head) {
                hit_block *hit = (hit_block *)pos->data;
                float dist_n = ivec3_distance(hit->b->origin_l, origin_p);

                if (pos == list->head) {
                        dist = dist_n;
                        ret = hit;
                        continue;
                }

                if (dist_n < dist) {
                        dist = dist_n;
                        ret = hit;
                        continue;
                }
        }

        return ret;
}

/**
 * grid_raytrace_2d() - Simple raytrace on 2D grid
 *
 * http://playtechs.blogspot.co.uk/2007/03/raytracing-on-grid.html
 *
 * Handy and tricky, but imprecise at line goes though block corner case.
 * Because our block is in another coordinate, we add block (x, y) here.
 *
 * @param x0: origin point x
 * @param y0: origin point y
 * @param x1: end point x
 * @param y1: end point y
 * @param o_x: origin block x
 * @param o_y: origin block y
 * @param ret: intersected block results
 */
void grid_raytrace_2d(float x0, float y0,
                      float x1, float y1,
                      int o_x, int o_y, linklist *ret)
{
        float dx = fabsf(x1 - x0);
        float dy = fabsf(y1 - y0);

        float dt_dx = 1.0f / dx;
        float dt_dy = 1.0f / dy;

        int x = (int)floorf(x0);
        int y = (int)floorf(y0);

        // Intersected grid count, sum of horizontal and vertical line crossings
        int n = 1;

        int x_inc, y_inc;
        float t_next_vertical, t_next_horizontal;

        if (float_equal(dx, 0.0f, FLT_EPSILON)) {
                x_inc = 0;
                t_next_horizontal = dt_dx; // inf
        } else if (x1 > x0) {
                x_inc = 1;
                n += (int)floorf(x1) - x;
                t_next_horizontal = (floorf(x0) + 1 - x0) * dt_dx;
        } else { // x1 < x0
                x_inc = -1;
                n += x - (int)floorf(x1);
                t_next_horizontal = (x0 - floorf(x0)) * dt_dx;
        }

        if (float_equal(dy, 0.0f, FLT_EPSILON)) {
                y_inc = 0;
                t_next_vertical = dt_dy;
        } else if (y1 > y0) {
                y_inc = 1;
                n += (int)floorf(y1) - y;
                t_next_vertical = (floorf(y0) + 1 - y0) * dt_dy;
        } else { // y1 < y0
                y_inc = -1;
                n += y - (int)floorf(y1);
                t_next_vertical = (y0 - floorf(y0)) * dt_dy;
        }

        for (; n > 0; n--) {
                ivec3 origin_b = { 0 };
                origin_b[X] = o_x;
                origin_b[Z] = o_y;

                linklist_append(ret, origin_b);

                if (t_next_vertical < t_next_horizontal) {
                        o_y += y_inc;
                        t_next_vertical += dt_dy;
                } else {
                        o_x += x_inc;
                        t_next_horizontal += dt_dx;
                }
        }
}

int player_ray_hittest(player *p, world *w, int radius)
{
        player_hittest *hittest = &p->hittest;
        linklist ray_blocks;
        linklist hit_blocks;
        ivec3 origin_pb = { 0 };
        vec3 point_end = { 0 };
        vec3 t = { 0 };

        linklist_init(&ray_blocks, sizeof(ivec3));
        linklist_init(&hit_blocks, sizeof(hit_block));

        // Get block position we at (round to integer)
        point_gl_to_local(p->origin_gl, BLOCK_EDGE_LEN_GLUNIT, t);
        vec3_round_ivec3(t, origin_pb);

        // Grid ray trace endpoint
        vec3_move(p->origin_gl, p->cam.vector_front, radius, point_end);

        // Vertically not rotated projection grid ray trace
        grid_raytrace_2d(p->origin_gl[X], p->origin_gl[Z],
                         point_end[X], point_end[Z],
                         origin_pb[X], origin_pb[Z], &ray_blocks);

        int h_min = clamp(origin_pb[Y] - radius, w->height_min, w->height_max);
        int h_max = clamp(origin_pb[Y] + radius, w->height_min, w->height_max);

        for (int h = h_min; h <= h_max; ++h) {
                block *b;
                block_face *f;
                linklist_node *pos;
                ivec3 origin_b = { 0 };

                linklist_for_each_node(pos, ray_blocks.head) {
                        origin_b[X] = (*(ivec3 *)pos->data)[X];
                        origin_b[Y] = h;
                        origin_b[Z] = (*(ivec3 *)pos->data)[Z];

                        if (!block_in_distance(origin_b, origin_pb, radius))
                                continue;

                        b = world_get_block(w, origin_b, L_NOWAIT);
                        if (b == NULL)
                                continue;

                        f = player_hittest_block_face(p, b);
                        if (!f)
                                continue;

                        linklist_append(&hit_blocks, &(hit_block){ .b = b, .f = f });
                }
        }

        if (linklist_is_empty(&hit_blocks)) {
                hittest->hit = 0;
                goto out;
        }

        hit_block *nearest = hit_block_nearest_get(&hit_blocks, origin_pb);

        hittest->hit = 1;
        hittest->face = nearest->f;
        ivec3_copy(nearest->b->origin_l, hittest->origin_b);

out:
        linklist_deinit(&ray_blocks);
        linklist_deinit(&hit_blocks);

        return hittest->hit;
}

void camera_vectors_compute(camera *cam, GLFWwindow *window, double speed)
{
        double x, y;
        double set_x, set_y;

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

void camera_perspective_compute(camera *cam)
{
        float fov = cam->zooming ? cam->fov_zoom : cam->fov;
        vec3 look_at = { 0, 0, 0 };

        glm_perspective(glm_rad(fov),
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
static inline int point_to_cube(const vec3 point_gl, int edge_len, ivec3 origin_l)
{
        if (__point_on_cube_edge(point_gl, edge_len))
                return 0;

        origin_l[X] = __point_to_cube(point_gl[X], edge_len);
        origin_l[Y] = __point_to_cube(point_gl[Y], edge_len);
        origin_l[Z] = __point_to_cube(point_gl[Z], edge_len);

        return 1;
}

/**
 * collision_test_block_point() - return block origin if point is in a block
 *
 * @param point_gl: point
 * @param origin_d: local coordinate of block detected collision
 * @return 1 on collision detected
 */
static inline int collision_test_block_point(const vec3 point_gl, ivec3 origin_d)
{
        ivec3 origin_b = { 0 };

        if (!point_to_cube(point_gl, BLOCK_EDGE_LEN_GLUNIT, origin_b)) {
                return 0;
        } else {
                ivec3_copy(origin_b, origin_d);
                return 1;
        }
}

void player_hitbox_vertices(vec3 *vertices, const vec3 origin, dimension size)
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

/**
 * player_collision_test() - perform player box collision test
 *
 * @param p: pointer to player
 * @param w: pointer to world to test blocks
 * @param origin_t: origin of test box
 * @return: 1 on collision detected
 */
int player_collision_test(player *p, world *w, vec3 origin_t)
{
        vec3 test_vertices[VERTICES_COLLISION_TEST];
        ivec3 origin_d = { 0 };

        player_hitbox_vertices(test_vertices, origin_t, p->size);

        for (int i = 0; i < VERTICES_COLLISION_TEST; ++i) {
                if (collision_test_block_point(test_vertices[i], origin_d)) {
                        if (world_get_block(w, origin_d, L_WAIT))
                                return 1;
                }
        }

        return 0;
}

int player_collision_test_single(player *p, ivec3 origin_b, vec3 origin_t)
{
        vec3 test_vertices[VERTICES_COLLISION_TEST];
        ivec3 origin_d = { 0 };

        player_hitbox_vertices(test_vertices, origin_t, p->size);

        for (int i = 0; i < VERTICES_COLLISION_TEST; ++i) {
                if (collision_test_block_point(test_vertices[i], origin_d)) {

                        // Same origin
                        if (!ivec3_cmp(origin_d, origin_b))
                                return 1;
                }
        }

        return 0;
}

/**
 * __player_move() - move player with collision test
 *
 * @param p: pointer to player
 * @param w: pointer to world to perform collision test with blocks
 * @param dir: moving direction vector
 * @param y: also move along Y axis
 * @param god: no clip, no collision test
 * @param ret: (X, Y, Z) axis collision test result
 */
void __player_move(player *p, world *w, const vec3 dir, int y, int god, ivec3 ret)
{
        // Temp origin to perform collision test
        vec3 o = { 0 };

        // Collision test results
        ivec3 c = { 0 };

        // X
        glm_vec_copy(p->origin_gl, o);
        o[X] += dir[X];

        if (!god)
                c[X] = player_collision_test(p, w, o);

        if (!c[X])
                p->origin_gl[X] = o[X];

        // Y
        if (y) {
                glm_vec_copy(p->origin_gl, o);
                o[Y] += dir[Y];

                if (!god)
                        c[Y] = player_collision_test(p, w, o);

                if (!c[Y])
                        p->origin_gl[Y] = o[Y];
        }

        // Z
        glm_vec_copy(p->origin_gl, o);
        o[Z] += dir[Z];

        if (!god)
                c[Z] = player_collision_test(p, w, o);

        if (!c[Z])
                p->origin_gl[Z] = o[Z];

        if (ret)
                ivec3_copy(c, ret);
}

static inline int player_is_standing(GLFWwindow *window)
{
        // Only on ground case: if (p->state == ONGROUND)

        if (glfwKeyReleased(window, GLFW_KEY_W) &&
            glfwKeyReleased(window, GLFW_KEY_S) &&
            glfwKeyReleased(window, GLFW_KEY_A) &&
            glfwKeyReleased(window, GLFW_KEY_D))
                return 1;
        else
                return 0;
}

void player_stand(player *p)
{
        p->speed.horizontal = 0;
}

void player_move(player *p, world *w, GLFWwindow *window)
{
        player_attr *attr = &p->attr;
        player_speed *speed = &p->speed;
        timestamp *ts = &player_ts;
        camera *cam = &p->cam;

        vec3 world_up = { 0.0f, 1.0f, 0.0f };
        vec3 horizontal_front = { 0 };
        vec3 vec_t = { 0 };

        if (p->state == INAIR) {
                if (glfwKeyReleased(window, GLFW_KEY_LEFT_SHIFT)) {
                        p->speed.horizontal = attr->air;
                }
        }

        if (p->state == ONGROUND) {
                if (player_is_standing(window)) {
                        player_stand(p);
                        return;
                }

                speed->horizontal = attr->walk;

                if (glfwKeyPressed(window, GLFW_KEY_LEFT_SHIFT))
                        speed->horizontal = attr->walk * attr->mod_sprint;

                if (glfwKeyPressed(window, GLFW_KEY_LEFT_CONTROL))
                        speed->horizontal = attr->walk * attr->mod_sneak;
        }

        // Vectors for forward/backward
        glm_cross(cam->vector_right, world_up, horizontal_front);
        glm_vec_scale(horizontal_front, ts->delta, vec_t);
        glm_vec_scale(vec_t, speed->horizontal, vec_t);
        glm_vec_inv(vec_t);

        // TODO: Move range check
        // TODO: Clamp

        // Forward
        if (glfwKeyPressed(window, GLFW_KEY_W)) {
                __player_move(p, w, vec_t, 0, 0, NULL);
        }

        // Backward
        if (glfwKeyPressed(window, GLFW_KEY_S)) {
                glm_vec_inv(vec_t);
                __player_move(p, w, vec_t, 0, 0, NULL);
        }

        // Vector for left/right
        glm_vec_scale(cam->vector_right, ts->delta, vec_t);
        glm_vec_scale(vec_t, speed->horizontal, vec_t);

        // Left
        if (glfwKeyPressed(window, GLFW_KEY_A)) {
                glm_vec_inv(vec_t);
                __player_move(p, w, vec_t, 0, 0, NULL);
        }

        // Right
        if (glfwKeyPressed(window, GLFW_KEY_D)) {
                __player_move(p, w, vec_t, 0, 0, NULL);
        }
}

static inline void player_land(player *p)
{
        p->state = ONGROUND;
        p->speed.vertical = 0;
}

static inline void player_fall(player *p)
{
        p->state = INAIR;
}

int player_is_on_ground(player *p, world *w)
{
        float dist_t = 0.03;
        vec3 origin_t = { 0 };

        glm_vec_copy(p->origin_gl, origin_t);

        origin_t[Y] -= dist_t;

        // Test whether we are on ground
        if (player_collision_test(p, w, origin_t)) {
                // Collision detected
                return 1;
        } else {
                return 0;
        }
}

void player_gravity_fall(player *p, world *w)
{
        player_speed *pspeed = &p->speed;
        timestamp *ts = &player_ts;
        vec3 world_up = { 0.0f, 1.0f, 0.0f };
        ivec3 collision = { 0 };
        vec3 vec_t = { 0 };

        if (p->state == INAIR) {
                pspeed->vertical -= WORLD_GRAVITY * ts->delta;

                glm_vec_scale(world_up, ts->delta, vec_t);
                glm_vec_scale(vec_t, pspeed->vertical, vec_t);

                __player_move(p, w, vec_t, 1, 0, collision);

                // Hit ground during falling
                if (collision[Y]) {
                        player_land(p);
                }
        }
}

void player_jump(player *p, GLFWwindow *window)
{
        player_speed *speed = &p->speed;
        player_attr *attr = &p->attr;

        // TODO: Update jump height

        if (glfwKeyPressed(window, GLFW_KEY_SPACE)) {
                if (p->state == ONGROUND) {
                        speed->vertical = attr->jump;

                        if (glfwKeyPressed(window, GLFW_KEY_LEFT_SHIFT)) {
                                speed->horizontal = attr->air * attr->mod_sprint;
                        }

                        p->state = INAIR;
                }
        }
}

void player_fly(player *p, world *w, GLFWwindow *window)
{
        player_attr *attr = &p->attr;
        timestamp *ts = &player_ts;
        camera *cam = &p->cam;

        vec3 world_up = { 0.0f, 1.0f, 0.0f };
        vec3 horizontal_front = { 0 };
        vec3 vec_t = { 0 };
        float speed = attr->fly;
        int god = attr->fly_noclip;

        if (glfwKeyPressed(window, GLFW_KEY_LEFT_SHIFT))
                speed *= attr->mod_sprint;

        // Vectors for forward/backward
        if (god) {
                glm_vec_scale(cam->vector_front, ts->delta, vec_t);
                glm_vec_scale(vec_t, speed, vec_t);
        } else {
                glm_cross(cam->vector_right, world_up, horizontal_front);
                glm_vec_scale(horizontal_front, ts->delta, vec_t);
                glm_vec_scale(vec_t, speed, vec_t);
                glm_vec_inv(vec_t);
        }

        // Forward
        if (glfwKeyPressed(window, GLFW_KEY_W)) {
                __player_move(p, w, vec_t, god, god, NULL);
        }

        // Backward
        if (glfwKeyPressed(window, GLFW_KEY_S)) {
                glm_vec_inv(vec_t);
                __player_move(p, w, vec_t, god, god, NULL);
        }

        // Vector for left/right
        glm_vec_scale(cam->vector_right, ts->delta, vec_t);
        glm_vec_scale(vec_t, speed, vec_t);

        // Left
        if (glfwKeyPressed(window, GLFW_KEY_A)) {
                glm_vec_inv(vec_t);
                __player_move(p, w, vec_t, 0, god, NULL);
        }

        // Right
        if (glfwKeyPressed(window, GLFW_KEY_D)) {
                __player_move(p, w, vec_t, 0, god, NULL);
        }

        // Vector for up/down
        glm_vec_scale(world_up, ts->delta, vec_t);
        glm_vec_scale(vec_t, speed, vec_t);

        // Up
        if (glfwKeyPressed(window, GLFW_KEY_SPACE)) {
                __player_move(p, w, vec_t, 1, god, NULL);
        }

        // Down
        if (glfwKeyPressed(window, GLFW_KEY_LEFT_CONTROL)) {
                glm_vec_inv(vec_t);
                __player_move(p, w, vec_t, 1, god, NULL);
        }
}

void player_movement_handle(player *p, world *w, GLFWwindow *window)
{
        player_movement state = p->state;

        switch (state) {
                case FLYING:
                        player_fly(p, w, window);
                        break;

                case ONGROUND:
                case INAIR:
                default:
                        if (player_is_on_ground(p, w)) {
                                player_land(p);
                        } else {
                                player_fall(p);
                        }

                        player_jump(p, window);
                        player_move(p, w, window);

                        player_gravity_fall(p, w);
                        break;
        }
}

void player_inputs_process(player *p, world *w, GLFWwindow *window)
{
        camera *cam = &p->cam;
        player_attr *attr = &p->attr;

        timestamp_update(&player_ts);

        player_movement_handle(p, w, window);

        camera_position_update(cam, p);
        camera_vectors_compute(cam, window, attr->view);
        camera_perspective_compute(cam);

        player_ray_hittest(p, w, attr->raycast_radius);
}

static inline void player_fly_switch(player *p)
{
        if (p->state == FLYING) {
                p->state = INAIR;
        } else {
                p->state = FLYING;
                p->speed.vertical = 0;
                p->speed.horizontal = 0;
        }
}

void player_action_destroy(player *p, world *w)
{
        player_hittest *hit_test = &p->hittest;

        if (!hit_test->hit)
                return;

        world_del_block(w, hit_test->origin_b);
}

void player_action_place(player *p, world *w)
{
        player_hittest *hit_test = &p->hittest;
        block_face *f = hit_test->face;
        ivec3 origin_new = { 0 };
        ivec3 f_normal = { 0 };
        block block_n;

        if (!hit_test->hit)
                return;

        vec3_round_ivec3(f->normal, f_normal);

        // Face normal must be normalized
        ivec3_add(hit_test->origin_b, f_normal, origin_new);

        if (player_collision_test_single(p, origin_new, p->origin_gl))
                return;

        block_init(&block_n, block_attr_get(player_item_current_get(p)), origin_new);
        world_add_block(w, &block_n, 1);
}

void player_key_callback(player *p, int key, int action)
{
        if (!p)
                return;

        if (key == GLFW_KEY_F && action == GLFW_PRESS) {
                player_fly_switch(p);
        }

        if (key == GLFW_KEY_C) {
                switch (action) {
                        case GLFW_PRESS:
                                p->cam.zooming = 1;
                                break;

                        case GLFW_RELEASE:
                                p->cam.zooming = 0;
                                break;
                }
        }
}

void player_mouse_callback(player *p, world *w, int button, int action)
{
        if (!p || !w)
                return;

        switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                        if (action == GLFW_RELEASE)
                                player_action_destroy(p, w);

                        break;

                case GLFW_MOUSE_BUTTON_RIGHT:
                        if (action == GLFW_RELEASE)
                                player_action_place(p, w);
                        break;

                default:
                        break;
        }
}

void player_scroll_callback(player *p, double offset_x, double offset_y)
{
        UNUSED_PARAM(p);
        UNUSED_PARAM(offset_x);

        player_item_scroll(p, offset_y);
}

static inline void player_attr_update(player *p)
{
        player_attr *attr = &p->attr;

        /*
         * v_t = v_0 + a * t
         * s = v_0 * t + (1/2) * a * (t ^ 2)
         *
         * a = g, s = h, v_0 = sqrt(2 * g * h)
         *
         */
        attr->jump = sqrtf(2 * WORLD_GRAVITY * attr->jump_height);
}

int player_position_set(player *p, vec3 pos)
{
        if (!p)
                return -EINVAL;

        glm_vec_copy(pos, p->origin_gl);

        camera_position_update(&p->cam, p);

        return 0;
}

int player_info_draw(player *p, int windows_width, int window_height)
{
        block_attr *blk_attr;
        vec3 origin_t = { 0 };
        ivec3 origin_b = { 0 };
        ivec3 origin_c = { 0 };
        char buf[256];

        blk_attr = block_attr_get(player_item_current_get(p));

        point_gl_to_local(p->origin_gl, BLOCK_EDGE_LEN_GLUNIT, origin_t);
        vec3_round_ivec3(origin_t, origin_b);
        block_in_chunk(origin_b, CHUNK_EDGE_LEN_GLUNIT, origin_c);

        sprintf_s(buf, sizeof(buf),
                  "Position: (%.4f %.4f %.4f)\n"
                  "Block: (%d %d %d) Chunk: (%d %d %d)\n"
                  "Item: %s",
                  p->origin_gl[X], p->origin_gl[Y], p->origin_gl[Z],
                  origin_b[X], origin_b[Y], origin_b[Z],
                  origin_c[X], origin_c[Y], origin_c[Z],
                  blk_attr->name);

        text_string_draw(buf, 0, 24, 1, NULL, NULL, 1, windows_width, window_height);

        return 0;
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

        timestamp_init(&player_ts);
        player_attr_update(p);
        player_item_init(p);

        return 0;
}

int player_deinit(player *p)
{
        if (!p)
                return -EINVAL;

        return 0;
}
