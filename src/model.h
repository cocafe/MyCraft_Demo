#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"

enum vec_attr {
        X = 0,
        Y,
        Z,
        W,
        NR_VEC_IDX,
};

enum vertices {
        V1 = 0,
        V2,
        V3,
        V4,
        V5,
        V6,
};

/* FIXME: The data size of cube vertex attributes are too large.
 * For example, 256^3 cubes can take up around 16GB memory
 * To reduce data size saves to disk, we can generate vertices
 * at runtime, but runtime memory consumption is still high.
 * Need optimizations.
 */
typedef struct cube_face {
        // TODO:        VBOs

        vec3            vertex[VERTICES_QUAD_FACE];
        vec3            vertex_normal[VERTICES_QUAD_FACE];
        vec2            uv[VERTICES_QUAD_FACE];

        vec3            normal;

        gl_attr         glattr;
} cube_face;

typedef struct model_cube {
        block           *block;

        vec3            origin; // World space

        // Orientation vectors
        vec3            axis[3];

        cube_face       faces[CUBE_FACES_QUADS];
} model_cube;

int model_cube_generate(model_cube *cube, block *block, vec3 origin /* front vector*/);
int model_cube_delete(model_cube *cube);
int model_cube_draw(model_cube *cube, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
