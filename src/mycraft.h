#ifndef MYCRAFT_DEMO_MYCRAFT_H
#define MYCRAFT_DEMO_MYCRAFT_H

#include "chunks.h"
#include "player.h"

#define PROGRAM_WINDOW_TITLE            ("MyCraft DEMO")
#define PROGRAM_WINDOW_WIDTH            (1280)
#define PROGRAM_WINDOW_HEIGHT           (720)

#define OPENGL_API_MAJOR                (3)
#define OPENGL_API_MINOR                (3)

#define VSYNC_ADAPTIVE                  (-1)
#define VSYNC_DISABLED                  (0)
#define VSYNC_ENABLED                   (1)

typedef struct mc_config {
        int32_t         vsync;
        int32_t         window_width;
        int32_t         window_height;
        int32_t         fullscreen;
        char            save_path[FILEPATH_MAX_LEN];
        float           fov;
        int32_t         render_dist;
        int32_t         crosshair_show;
        int32_t         texture_filter_level;
        int32_t         texture_mipmap_level;
        uint32_t        debug_level;
        int32_t         opengl_msaa;
        int32_t         cursor_speed;
        int32_t         show_fps;
        int32_t         no_clip;
} mc_config;

typedef enum program_state {
        PROGRAM_UNKNOWN = 0,
        PROGRAM_INITED,
        PROGRAM_RUNNING,
        PROGRAM_PAUSE,
        PROGRAM_EXIT,
} program_state;

typedef struct mc_program {
        mc_config       config;

        GLFWwindow      *window;
        int32_t         window_width;
        int32_t         window_height;

        GLuint          VAO;
        fps_meter       fps;

        world           mc_world;
        player          mc_player;

        program_state   state;
        int             focus;
} mc_program;

extern mc_program *g_program;

#endif //MYCRAFT_DEMO_MYCRAFT_H
