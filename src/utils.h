#ifndef MYCRAFT_DEMO_UTIL_H
#define MYCRAFT_DEMO_UTIL_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

/**
 * Compiler
 */
#define ARRAY_SIZE(a)                   (sizeof(a) / sizeof((a)[0]))

#if defined(__GNUC__)
#define likely(x)                       __builtin_expect((x), 1)
#define unlikely(x)                     __builtin_expect((x), 0)
#else
#define likely(x)                       (x)
#define unlikely(x)                     (x)
#endif

#define UNUSED_PARAM(x)                 (void)(x)

/**
 * Simple Time Profiler
 */
#define SEC_TO_MS(s)                    ((s) * 1000)

// XXX: Be careful with variable scope

#define TIME_PROFILER_SET                               \
        double profiler_timestamp = glfwGetTime()

#define TIME_PROFILER_RET                               \
        pr_debug_func("profiler: %lf ms\n",             \
        SEC_TO_MS(glfwGetTime() - profiler_timestamp))

/**
 * Resources Paths
 */
#define RESOURCES_PATH                  "./resource/"
#define RESOURCES_SHADER_PATH           RESOURCES_PATH "shader/"
#define RESOURCES_TEXTURE_PATH          RESOURCES_PATH "texture/"

#define SHADER_FILE(file)               (RESOURCES_SHADER_PATH file SUFFIX_GLSL)
#define TEXTURE_PNG(file)               (RESOURCES_TEXTURE_PATH file SUFFIX_PNG)

/**
 * File
 */
#define FILEPATH_MAX_LEN                (1024)

#define SUFFIX_PNG                      ".png"
#define SUFFIX_BMP                      ".bmp"
#define SUFFIX_TGA                      ".tga"
#define SUFFIX_DDS                      ".dds"
#define SUFFIX_OBJ                      ".obj"
#define SUFFIX_GLSL                     ".glsl"

/**
 * Vertex
 */
#define VERTICES_LINE                   (2)
#define VERTICES_TRIANGLE               (3)
#define VERTICES_QUAD                   (4)
#define VERTICES_CUBE                   (8)
#define VERTICES_TRIANGULATE_QUAD       (2 * VERTICES_TRIANGLE)
#define VERTICES_TRIANGULATE_CUBE       (CUBE_QUAD_FACES * VERTICES_TRIANGULATE_QUAD)

enum vec3_attr {
        X = 0,
        Y,
        Z,
        NR_VEC3_ATTR,
};

enum vec4_attr {
        W = 3,
};

enum vertex_idx {
        V1 = 0,
        V2,
        V3,
        V4,
        V5,
        V6,
        V7,
        V8,
        V9,
        V10,
        V11,
        V12,
};

typedef float vec2[2];

typedef int32_t ivec2[2];
//typedef int32_t ivec3[3];     // cglm has defined one
typedef int32_t ivec4[4];

typedef float *pvec2;
typedef float *pvec3;
typedef float *pvec4;

/**
 * Line
 */

#define LINES_TRIANGLE                  (3)
#define LINES_QUAD                      (4)
#define LINES_CUBE                      (12)

typedef vec2 line_vec2[VERTICES_LINE];
typedef vec3 line_vec3[VERTICES_LINE];

/**
 * Face
 */

#define CUBE_QUAD_FACES                 (6)

/**
 * Vector Operations
 */

void ivec3_copy(const ivec3 src, ivec3 dst);
float ivec3_distance(ivec3 v1, ivec3 v2);
void vec3_round_ivec3(const vec3 src, ivec3 dst);

/**
 * Equality Test
 */

int vec2_equal(const vec2 a, const vec2 b);
int vec3_equal(const vec3 a, const vec3 b);
int ivec2_equal(const ivec2 a, const ivec2 b);
int ivec3_equal(const ivec3 a, const ivec3 b);
int float_equal(float a, float b, float epsilon);

/**
 * Math
 */

int clamp(int x, int min, int max);
float clampf(float x, float min, float max);
double clamplf(double x, double min, double max);

int cycle(int x, int bound_lower, int bound_upper);
float cyclef(float x, float bound_lower, float bound_upper);
double cyclelf(double x, double bound_lower, double bound_upper);

/**
 * Memory Heap
 */
int memzero(void *ptr, size_t size);
int memdump(void *ptr, size_t size);
void *memalloc(size_t size);
int memfree(void **ptr);

/**
 * File
 */
char *file_read(const char *filepath);

/**
 * String Buffer
 */
char *buf_alloc(size_t len);
int buf_free(char **buf);

/**
 * Misc
 */

// pthread lock wait flag
enum {
        L_NOWAIT = 0,
        L_WAIT,
};

long get_cpu_count(void);
void image_vertical_flip(uint8_t *data, uint32_t width, uint32_t height);

typedef struct timestamp {
        double  last;
        double  curr;
        float   delta;
} timestamp;

int timestamp_init(timestamp *t);
int timestamp_update(timestamp *t);

int glfwKeyPressed(GLFWwindow *window, int key);
int glfwKeyReleased(GLFWwindow *window, int key);

/**
 * Sequence List Implementation
 */

typedef struct seqlist {
        void    *data;
        size_t  element_size;

        size_t  count_expand;
        size_t  count_utilized;
        size_t  count_allocated;
} seqlist;

int seqlist_alloc(seqlist **list);
int seqlist_free(seqlist **list);
int seqlist_init(seqlist *list, size_t element_size, size_t count);
int seqlist_deinit(seqlist *list);
int seqlist_expand(seqlist *list, size_t count);
int seqlist_shrink(seqlist *list);
int seqlist_append(seqlist *list, void *element);
int seqlist_is_empty(seqlist *list);

/**
 * Linked List Implementation
 */

typedef struct linklist_node {
        void                    *data;
        // TODO: We can make it custom attribute struct
        int                     flag_delete;

        struct linklist_node    *prev;
        struct linklist_node    *next;
} linklist_node;

int linklist_node_alloc(linklist_node **n);
int linklist_node_free(linklist_node **n);
int linklist_node_init(linklist_node *n, linklist_node *prev,
                       linklist_node *next, void *element, size_t element_size);
int linklist_node_deinit(linklist_node *n);

typedef struct linklist {
        linklist_node           *head;
        size_t                  element_size;
        size_t                  element_count;

        // FIXME: Not thread-safe
} linklist;

#define linklist_for_each_node(pos, head) \
        for ((pos) = (head); (pos) != NULL; (pos) = (pos)->next)

int linklist_alloc(linklist **list);
int linklist_free(linklist **list);
int linklist_init(linklist *list, size_t element_size);
int linklist_deinit(linklist *list);
void *linklist_append(linklist *list, void *element);
int linklist_delete(linklist *list, linklist_node *node);
int linklist_delete_marked(linklist *list);
int linklist_is_empty(linklist *list);

#endif //MYCRAFT_DEMO_UTIL_H
