#ifndef MYCRAFT_DEMO_PLAYER_H
#define MYCRAFT_DEMO_PLAYER_H

#include <pthread.h>

#include <cglm/cglm.h>

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

        double                  time_last;
        double                  time_curr;
        float                   time_delta;

        mat4                    mat_persp;
        mat4                    mat_camera;
        mat4                    mat_transform;
} camera;

typedef enum move_direction {
        MOVE_FORWARD = 0,
        MOVE_BACKWARD,
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN,
        NR_MOVE_DIRECTION,
} move_direction;

typedef enum player_movement {
        STANDING = 0,
        WALKING,
        RUNNING,
        SNEAKING,      // TODO: Height changes dynamically
        FLYING,
        FALLING,
        NR_PLAYER_MOVEMENT,
} player_movement;

typedef struct player_speed {
        float fly;
        float fly_sprint;

        float walk;
        float sprint;
        float sneak;

        float jump;
        float jump_air;   // TODO: Sprinted jump

        float view;
} player_speed;

typedef struct player {
        dimension       size;
        vec3            origin_gl;

        camera          cam;
        vec3            cam_offset;     // Position relative to player origin

        player_movement state;
        player_speed    speed;
} player;

void player_inputs_handle(player *p, world *w, GLFWwindow *window);

int player_hint(player *p, player *hint);
int player_default(player *p);
int player_init(player *p);
int player_deinit(player *p);

#endif //MYCRAFT_DEMO_PLAYER_H
