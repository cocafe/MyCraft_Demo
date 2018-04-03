#ifndef MYCRAFT_DEMO_UTIL_H
#define MYCRAFT_DEMO_UTIL_H

#include <cglm/cglm.h>

#define ARRAY_SIZE(a)                   (sizeof(a) / sizeof((a)[0]))

#if defined(__GNUC__)
#define likely(x)                       __builtin_expect((x), 1)
#define unlikely(x)                     __builtin_expect((x), 0)
#else
#define likely(x)                       (x)
#define unlikely(x)                     (x)
#endif

#define FILEPATH_MAX_LEN                (1024)

#define SUFFIX_PNG                      ".png"
#define SUFFIX_BMP                      ".bmp"
#define SUFFIX_TGA                      ".tga"
#define SUFFIX_DDS                      ".dds"
#define SUFFIX_OBJ                      ".obj"
#define SUFFIX_GLSL                     ".glsl"

#define VERTICES_TRIANGLE               (3)
#define VERTICES_QUAD                   (4)
#define VERTICES_TRIANGULATE_QUAD       (2 * VERTICES_TRIANGLE)

enum vec_attr {
        X = 0,
        Y,
        Z,
        W,
        NR_VEC_IDX,
};

enum vertex_idx {
        V1 = 0,
        V2,
        V3,
        V4,
        V5,
        V6,
};

typedef float vec2[2];

typedef int32_t ivec2[2];
//typedef int32_t ivec3[3];     // cglm has defined one
typedef int32_t ivec4[4];

typedef float *pvec2;
typedef float *pvec3;
typedef float *pvec4;

int ivec3_equal(const ivec3 a, const ivec3 b);
int float_equal(float a, float b, float epsilon);

int clamp(int x, int min, int max);
float clampf(float x, float min, float max);
double clamplf(double x, double min, double max);

int cycle(int x, int bound_lower, int bound_upper);
float cyclef(float x, float bound_lower, float bound_upper);
double cyclelf(double x, double bound_lower, double bound_upper);

int memzero(void *ptr, size_t size);
int memdump(void *ptr, size_t size);
void *memalloc(size_t size);
int memfree(void **ptr);

char *file_read(const char *filepath);

char *buf_alloc(size_t len);
int buf_free(char **buf);

void image_vertical_flip(uint8_t *data, uint32_t width, uint32_t height);

/**
 * Sequence List Implementation
 */

#define SEQLIST_EXPAND_COUNT           (8)

typedef struct seqlist {
        void    *data;
        size_t  element_size;

        size_t  count_seqlist;
        size_t  count_utilized;
} seqlist;

int seqlist_alloc(seqlist **list);
int seqlist_free(seqlist **list);
int seqlist_init(seqlist *list, size_t element_size, size_t count);
int seqlist_deinit(seqlist *list);
int seqlist_expand(seqlist *list, size_t count);
int seqlist_shrink(seqlist *list);
int seqlist_append(seqlist *list, void *element);

/**
 * Linked List Implementation
 */

typedef struct linklist_node {
        void                    *data;
        // TODO: We can make it custom attribute struct
        int                     mark_delete;

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
