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
#define FOV_ZOOMING                     (15.0f)

typedef struct camera {
        vec3                    position;

        float                   fov;
        float                   fov_zoom;
        int                     zooming;

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

typedef struct player_attr {
        float   fly;
        int     fly_noclip;

        float   air;
        float   walk;

        float   jump;
        float   jump_height;

        float   mod_sprint;
        float   mod_sneak;

        float   view;

        int32_t raycast_radius;
} player_attr;

typedef struct player_hittest {
        int             hit;
        ivec3           origin_b;
        block_face      *face;
} player_hittest;

typedef struct player_item {
        block_attr_idx  *items;
        int             count;
        int             current;
        // TODO: item type
} player_item;

typedef struct player {
        dimension               size;
        vec3                    origin_gl;

        camera                  cam;
        // Position relative to player origin
        vec3                    cam_offset;

        player_movement         state;
        player_hittest          hittest;
        player_speed            speed;
        player_attr             attr;
        player_item             item;
} player;

void player_inputs_process(player *p, world *w, GLFWwindow *window);

void player_key_callback(player *p, int key, int action);
void player_mouse_callback(player *p, world *w, int button, int action);
void player_scroll_callback(player *p, double offset_x, double offset_y);

int player_position_set(player *p, vec3 pos);
int player_position_show(player *p, int windows_width, int window_height);

int player_hint(player *p, player *hint);
int player_default(player *p);
int player_init(player *p);
int player_deinit(player *p);

#endif //MYCRAFT_DEMO_PLAYER_H
