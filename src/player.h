#ifndef MYCRAFT_DEMO_PLAYER_H
#define MYCRAFT_DEMO_PLAYER_H

#include <cglm/cglm.h>

typedef struct camera {
        vec3            position;
        float           fov;
        float           fov_delta;
        int32_t         window_width;
        int32_t         window_height;
        double          angel_horizontal;
        double          angel_vertical;
        mat4            mat_persp;
        mat4            mat_camera;
        mat4            mat_transform;
        double          time_last;
        double          time_curr;
        float           time_delta;
        float           clamp_near;
        float           clamp_far;
        vec3            vector_front;
        vec3            vector_up;
        vec3            vector_right;
        // TODO: mutex lock / RCU lock
        // TODO: TPS camera --> ray hittest
} camera;

void camera_inputs_handle(GLFWwindow *window, camera *cam,
                          double speed_move, double speed_view);

#endif //MYCRAFT_DEMO_PLAYER_H
