#ifndef MYCRAFT_DEMO_PLAYER_H
#define MYCRAFT_DEMO_PLAYER_H

#include <pthread.h>

#include <cglm/cglm.h>

#define PLAYER_HEIGHT                   (1.95f * BLOCK_EDGE_LEN_GLUNIT)
#define PLAYER_LENGTH                   (0.5f * BLOCK_EDGE_LEN_GLUNIT)
#define PLAYER_WIDTH                    (0.5f * BLOCK_EDGE_LEN_GLUNIT)

#define FOV_MINIMAL                     (30.0f)
#define FOV_NORMAL                      (70.0f)
#define FOV_MAXIMUM                     (120.0f)

typedef struct camera {
        vec3                    position;

        float                   fov;
        float                   fov_delta;

        int32_t                 view_width;
        int32_t                 view_height;

        double                  angel_horizontal;
        double                  angel_vertical;

        float                   clamp_near;
        float                   clamp_far;

        vec3                    vector_front;
        vec3                    vector_up;
        vec3                    vector_right;

        mat4                    mat_persp;
        mat4                    mat_camera;
        mat4                    mat_transform;
} camera;

typedef enum player_movement {
        INAIR = 0,
        ONGROUND,
        FLYING,
        NR_PLAYER_MOVEMENT,
} player_movement;

typedef struct player_speed {
        float vertical;
        float horizontal;
} player_speed;

typedef struct player_speed_sets {
        float fly;
        float air;
        float walk;

        float jump;
        float jump_height;

        float mod_sprint;
        float mod_sneak;

        float view;
} player_speed_sets;

typedef struct player {
        dimension               size;
        vec3                    origin_gl;

        camera                  cam;
        // Position relative to player origin
        vec3                    cam_offset;

        player_movement         state;
        player_speed            speed;
        player_speed_sets       speed_sets;
} player;

void player_movement_perform(player *p, world *w, GLFWwindow *window);

int player_position_set(player *p, vec3 pos);

int player_hint(player *p, player *hint);
int player_default(player *p);
int player_init(player *p);
int player_deinit(player *p);

#endif //MYCRAFT_DEMO_PLAYER_H
