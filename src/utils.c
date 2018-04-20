#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

#include "debug.h"
#include "utils.h"

extern inline void vec3_to_vec4(const vec3 src, float w, vec4 dst)
{
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = w;
}

extern inline void vec4_to_vec3(const vec4 src, vec3 dst)
{
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
}

extern inline void ivec3_add(const ivec3 a, const ivec3 b, ivec3 dst)
{
        dst[0] = a[0] + b[0];
        dst[1] = a[1] + b[1];
        dst[2] = a[2] + b[2];
}

extern inline void ivec3_copy(const ivec3 src, ivec3 dst)
{
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
}

extern inline float ivec3_distance(ivec3 v1, ivec3 v2)
{
        return sqrtf(glm_pow2(v2[0] - v1[0]) +
                     glm_pow2(v2[1] - v1[1]) +
                     glm_pow2(v2[2] - v1[2]));
}

extern inline void vec3_round_ivec3(const vec3 src, ivec3 dst)
{
        dst[0] = (int)(roundf(src[0]));
        dst[1] = (int)(roundf(src[1]));
        dst[2] = (int)(roundf(src[2]));
}

int vec2_equal(const vec2 a, const vec2 b)
{
        float epsilon = FLT_EPSILON;

        if (float_equal(a[0], b[0], epsilon) &&
            float_equal(a[1], b[1], epsilon))
                return 1;
        else
                return 0;
}

int vec3_equal(const vec3 a, const vec3 b)
{
        float epsilon = FLT_EPSILON;

        if (float_equal(a[0], b[0], epsilon) &&
            float_equal(a[1], b[1], epsilon) &&
            float_equal(a[2], b[2], epsilon))
                return 1;
        else
                return 0;
}

int ivec2_equal(const ivec2 a, const ivec2 b)
{
        for (int i = 0; i < 2; ++i) {
                if (a[i] != b[i])
                        return 0;
        }

        return 1;
}

int ivec3_equal(const ivec3 a, const ivec3 b)
{
        for (int i = 0; i < 3; ++i) {
                if (a[i] != b[i])
                        return 0;
        }

        return 1;
}

#if 0
// Nearly equal
int float_equal(float a, float b, float epsilon)
{
        float abs_a = fabsf(a);
        float abs_b = fabsf(b);
        float diff = fabsf(a - b);

        if (a == b) {
                return true;
        } else if (a == 0 || b == 0 || diff < FLT_MIN){
                return diff < (epsilon * FLT_MIN);
        } else {
                return (diff / fminf((abs_a + abs_b), FLT_MIN)) < epsilon;
        }
}
#else
// Not precise, and not handling edge case
int float_equal(float a, float b, float epsilon)
{
        if (a == b)
                return true;

        return fabsf(a - b) <= epsilon;
}
#endif

/**
 * clamp() - restrict value to given range [min, max]
 *
 * @param x: value to clamp
 * @param min: min value
 * @param max: max value
 * @return clamped value
 */
int clamp(int x, int min, int max)
{
        if (x < min)
                x = min;
        else if (x > max)
                x = max;

        return x;
}

float clampf(float x, float min, float max)
{
        if (x < min)
                x = min;
        else if (x > max)
                x = max;

        return x;
}

double clamplf(double x, double min, double max)
{
        if (x < min)
                x = min;
        else if (x > max)
                x = max;

        return x;
}

/**
 * cycle() - set value to inverse bound when exceeds
 *
 * @param x: value to set
 * @param bound_lower: lower bound
 * @param bound_upper: upper bound
 * @return computed value
 */
int cycle(int x, int bound_lower, int bound_upper)
{
        if (x < bound_lower)
                x = bound_upper;
        else if (x > bound_upper)
                x = bound_lower;

        return 0;
}

float cyclef(float x, float bound_lower, float bound_upper)
{
        if (x < bound_lower)
                x = bound_upper;
        else if (x > bound_upper)
                x = bound_lower;

        return 0;
}

double cyclelf(double x, double bound_lower, double bound_upper)
{
        if (x < bound_lower)
                x = bound_upper;
        else if (x > bound_upper)
                x = bound_lower;

        return x;
}

int memzero(void *ptr, size_t size)
{
        if (!ptr)
                return -EINVAL;

        memset(ptr, 0x00, size);

        return 0;
}

int memdump(void *ptr, size_t size)
{
        uint8_t *p = (uint8_t *)ptr;
        int column = 8;

        if (!ptr)
                return -EINVAL;

        fprintf(stdout, "     ");

        for (int i = 0; i < column; ++i) {
                fprintf(stdout, " %d ", i);
        }

        fprintf(stdout, "\n");

        for (uint32_t i = 0, l = 1; i < size; i++) {
                if (l == 0 || ((i % column) == 0)) {
                        fprintf(stdout, "%4d ", l);
                }

                fprintf(stdout, "%02x ", p[i]);

                if (((i + 1) % column) == 0) {
                        fprintf(stdout, "\n");
                        l++;
                }

                fflush(stdout);
        }

        return 0;
}

//int memalloc(void **ptr, size_t size)
//{
//        if (!ptr)
//                return -EINVAL;
//
//        *ptr = calloc(1, size);
//        if (!*ptr)
//                return -ENOMEM;
//
//        return 0;
//}

void *memalloc(size_t size)
{
        void *ptr = calloc(1, size);

        return ptr;
}

int memfree(void **ptr)
{
        if (!ptr)
                return -EINVAL;

        if (!*ptr)
                return -ENODATA;

        free(*ptr);
        *ptr = NULL;

        return 0;
}

char *file_read(const char *filepath)
{
        errno_t err;
        FILE *fp;
        char *buf;
        char ch;
        size_t fsize;
        size_t i;

        err = fopen_s(&fp, filepath, "r");
        if (!fp) {
                pr_err_fopen(filepath, err);
                return NULL;
        }

        fsize = 0;
        while (1) {
                ch = (char)fgetc(fp);

                if (ferror(fp)) {
                        pr_err_func("failed to read file %s\n", filepath);
                        return NULL;
                }

                if (feof(fp)) {
                        break;
                }

                fsize++;
        }

        buf = calloc(1, sizeof(char) * (fsize + 2));
        if (!buf) {
                pr_err_alloc();
                return NULL;
        }

        memset(buf, '\0', sizeof(char) * (fsize + 2));

        rewind(fp);
        i = 0;
        while (1) {
                ch = (char)fgetc(fp);

                if (ferror(fp)) {
                        pr_err_func("failed to read file %s\n", filepath);
                        return NULL;
                }

                if (feof(fp)) {
                        break;
                }

                buf[i] = ch;
                i++;
        }

        return buf;
}

char *buf_alloc(size_t len)
{
        if (!len)
                return NULL;

        char *buf = calloc(1, sizeof(char) * len);
        if (!buf)
                return NULL;

        memset(buf, '\0', sizeof(char) * len);

        return buf;
}

int buf_free(char **buf)
{
        if (!*buf)
                return -EINVAL;

        free(*buf);
        *buf = NULL;

        return 0;
}

long get_cpu_count(void)
{
#ifdef __MINGW32__
        SYSTEM_INFO sysinfo;

        GetSystemInfo(&sysinfo);

        return sysinfo.dwNumberOfProcessors;
#else /* POSIX extension */
        return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void image_vertical_flip(uint8_t *data, uint32_t width, uint32_t height)
{
        size_t size = width * height * 4;
        uint32_t stride = sizeof(int8_t) * width * 4;
        uint8_t *new_data = calloc(1, sizeof(uint8_t) * size);

        for (uint32_t i = 0; i < height; i++) {
                int j = height - i - 1;
                memcpy(new_data + j * stride, data + i * stride, stride);
        }

        memcpy(data, new_data, size);
        free(new_data);
}

int glfwKeyPressed(GLFWwindow *window, int key)
{
        return (glfwGetKey(window, key) == GLFW_PRESS);
}

int glfwKeyReleased(GLFWwindow *window, int key)
{
        return (glfwGetKey(window, key) == GLFW_RELEASE);
}

/**
 * Time Stamp
 */

int timestamp_init(timestamp *t)
{
        if (!t)
                return -EINVAL;

        memzero(t, sizeof(timestamp));

        t->curr = glfwGetTime();
        t->last = t->curr;

        return 0;
}

int timestamp_update(timestamp *t)
{
        if (!t)
                return -EINVAL;

        t->curr = glfwGetTime();
        t->delta = (float)(t->curr - t->last);
        t->last = t->curr;

        return 0;
}

/**
 * Sequence List Implementation
 */

int seqlist_alloc(seqlist **list)
{
        if (!list)
                return -EINVAL;

        *list = calloc(1, sizeof(seqlist));
        if (!*list) {
                pr_err_alloc();
                return -ENOMEM;
        }

        return 0;
}

int seqlist_free(seqlist **list)
{
        if (!list)
                return -EINVAL;

        if (!*list)
                return -ENODATA;

        free(*list);
        *list = NULL;

        return 0;
}

int seqlist_init(seqlist *list, size_t element_size, size_t count)
{
        if (!list)
                return 0;

        memzero(list, sizeof(seqlist));

        list->data = calloc(count, element_size);
        if (!list->data) {
                pr_err_alloc();
                return -ENOMEM;
        }

        list->element_size = element_size;
        list->count_allocated = count;
        list->count_utilized = 0;
        list->count_expand = count;

        pthread_spin_init(&list->spinlock, PTHREAD_PROCESS_PRIVATE);

        return 0;
}

int seqlist_deinit(seqlist *list)
{
        if (!list)
                return -EINVAL;

        if (!list->data)
                return -ENODATA;

        pthread_spin_lock(&list->spinlock);

        free(list->data);

        pthread_spin_unlock(&list->spinlock);

        pthread_spin_destroy(&list->spinlock);

        memzero(list, sizeof(seqlist));

        return 0;
}

/**
 * seqlist_expand() - expand sequence list, internal call
 *
 * @param list: pointer to list
 * @param count: element count to expand
 * @return 0 on success
 */
int seqlist_expand(seqlist *list, size_t count)
{
        void *new_data;
        size_t new_count;

        new_count = list->count_allocated + count;

        /*
         * We do not use realloc() here, realloc() does not guarantee
         * re-allocated memory block is clear, and we do some bytes copies
         * in seqlist_append()
         */
        new_data = calloc(new_count, list->element_size);
        if (!new_data) {
                pr_err_alloc();
                return -ENOMEM;
        }

        memcpy(new_data, list->data, list->element_size * list->count_utilized);
        free(list->data);

        list->data = new_data;
        list->count_allocated = new_count;

        return 0;
}

int seqlist_shrink(seqlist *list)
{
        void *new_data;

        if (!list)
                return -EINVAL;

        if (seqlist_is_empty(list))
                return -ENODATA;

        pthread_spin_lock(&list->spinlock);

        new_data = realloc(list->data, list->element_size * list->count_utilized);
        if (!new_data) {
                pr_err_alloc();
                goto out;
        }

        list->data = new_data;
        list->count_allocated = list->count_utilized;

out:
        pthread_spin_unlock(&list->spinlock);

        return 0;
}

int seqlist_append(seqlist *list, void *element)
{
        uint8_t *t;
        size_t i;
        size_t element_size;

        if (!list || !element)
                return -EINVAL;

        pthread_spin_lock(&list->spinlock);

        if (list->count_utilized >= list->count_allocated) {
                if (seqlist_expand(list, list->count_expand)) {
                        pr_err_func("failed to expand sequence list\n");
                        goto out;
                }
        }

        // XXX: Memory block pointer is changed after seqlist_expand()
        t = (uint8_t *)list->data;
        i = list->count_utilized;
        element_size = list->element_size;

        memcpy(&t[(element_size / sizeof(uint8_t)) * i], element, element_size);

        list->count_utilized++;

out:
        pthread_spin_unlock(&list->spinlock);

        return 0;
}

/**
 * seqlist_is_empty() - check list whether empty
 * @param list: pointer to list
 * @return 1 on empty
 */
int seqlist_is_empty(seqlist *list)
{
        int ret = 0;

        if (!list)
                return 1;

        if (!list->data)
                return 1;

        pthread_spin_lock(&list->spinlock);

        if (list->count_utilized == 0)
                ret = 1;

        pthread_spin_unlock(&list->spinlock);

        return ret;
}

/**
 * Linked List Implementation
 */

int linklist_node_alloc(linklist_node **n)
{
        if (!n)
                return -EINVAL;

        *n = calloc(1, sizeof(linklist_node));
        if (!*n) {
                pr_err_alloc();
                return -ENOMEM;
        }

        return 0;
}

int linklist_node_free(linklist_node **n)
{
        if (!n)
                return -EINVAL;

        if (!*n)
                return -ENODATA;

        free(*n);
        *n = NULL;

        return 0;
}

int linklist_node_init(linklist_node *n, linklist_node *prev,
                       linklist_node *next, void *element, size_t element_size)
{
        if (!n || !element)
                return -EINVAL;

        n->data = calloc(1, element_size);
        if (!n->data) {
                pr_err_alloc();
                return -ENOMEM;
        }

        // memcpy(n->data, element, sizeof(*element));
        memcpy(n->data, element, element_size);

        n->prev = prev;
        n->next = next;
        n->flag_delete = 0;

        return 0;
}

int linklist_node_deinit(linklist_node *n)
{
        if (!n)
                return -EINVAL;

        if (!n->data)
                return -ENODATA;

        free(n->data);
        n->data = NULL;

        return 0;
}

int linklist_alloc(linklist **list)
{
        if (!list)
                return -EINVAL;

        *list = calloc(1, sizeof(linklist));
        if (!*list) {
                pr_err_alloc();
                return -ENOMEM;
        }

        return 0;
}

int linklist_free(linklist **list)
{
        if (!list)
                return -EINVAL;

        if (!*list)
                return -ENODATA;

        free(*list);
        *list = NULL;

        return 0;
}

int linklist_init(linklist *list, size_t element_size)
{
        if (!list)
                return -EINVAL;

        memzero(list, sizeof(linklist));

        list->element_size = element_size;
        list->element_count = 0;

        pthread_spin_init(&list->spinlock, PTHREAD_PROCESS_PRIVATE);

        return 0;
}

int linklist_deinit(linklist *list)
{
        linklist_node *curr, *next;

        if (!list)
                return -EINVAL;

        if (!list->head)
                return -ENODATA;

        pthread_spin_lock(&list->spinlock);

        curr = list->head;
        while (curr) {
                next = curr->next;

                linklist_node_deinit(curr);
                linklist_node_free(&curr);

                curr = next;
        }

        pthread_spin_unlock(&list->spinlock);

        pthread_spin_destroy(&list->spinlock);

        memzero(list, sizeof(linklist));

        return 0;
}

/**
 * linklist_append() - copy and add element to linked list
 *
 * @param list
 * @param element
 * @return copied data pointer
 */
void *linklist_append(linklist *list, void *element)
{
        linklist_node *curr, *node;
        void *ret = NULL;

        if (!list || !element)
                return NULL;

        pthread_spin_lock(&list->spinlock);

        if (linklist_node_alloc(&node))
                goto out;

        if (linklist_node_init(node, NULL, NULL, element, list->element_size)) {
                linklist_node_free(&node);
                goto out;
        }

        ret = node->data;

        if (list->head == NULL) {
                list->head = node;
                list->element_count++;

                goto out;
        }

        // Move to last valid node
        curr = list->head;
        while (curr->next)
                curr = curr->next;

        curr->next = node;
        curr->next->prev = curr;

        list->element_count++;

out:
        pthread_spin_unlock(&list->spinlock);

        return ret;
}

static inline void __linklist_node_delete(linklist *list, linklist_node **n)
{
        linklist_node_deinit(*n);
        linklist_node_free(n);

        list->element_count--;
}

/**
 * linklist_delete() - delete the node address which matches
 *
 * single mode, delete one node in one loop
 *
 * @param list: pointer to linked list
 * @param node: pointer to match and delete
 * @return 0 on success
 */
int linklist_delete(linklist *list, linklist_node *node)
{
        linklist_node *curr, *prev, *next;

        if (!list)
                return -EINVAL;

        if (!list->head)
                return -ENODATA;

        pthread_spin_lock(&list->spinlock);

        if (list->head == node) {
                next = list->head->next;

                __linklist_node_delete(list, &list->head);

                list->head = next;
                if (list->head != NULL)
                        list->head->prev = NULL;

                goto out;
        }

        curr = list->head;
        while (curr) {
                prev = curr->prev;
                next = curr->next;

                if (curr == node) {
                        prev->next = next;
                        if (next != NULL)
                                next->prev = prev;

                        __linklist_node_delete(list, &curr);

                        break;
                }

                curr = next;
        }

out:
        pthread_spin_unlock(&list->spinlock);

        return 0;
}

/**
 * linklist_delete_marked() - delete all nodes that are marked delete flag
 *
 * batch mode, delete nodes in one loop
 *
 * @param list: pointer to linked list
 * @return 0 on success
 */
int linklist_delete_marked(linklist *list)
{
        linklist_node *curr, *prev, *next;

        if (!list)
                return -EINVAL;

        if (!list->head)
                return -ENODATA;

        pthread_spin_lock(&list->spinlock);

        while (list->head && list->head->flag_delete) {
                next = list->head->next;

                __linklist_node_delete(list, &list->head);

                list->head = next;
                if (list->head != NULL)
                        list->head->prev = NULL;
        }

        curr = list->head;
        while (curr) {
                prev = curr->prev;
                next = curr->next;

                if (curr->flag_delete) {
                        prev->next = next;
                        if (next != NULL)
                                next->prev = prev;

                        __linklist_node_delete(list, &curr);

                        curr = prev;
                        continue;
                }

                curr = next;
        }

        pthread_spin_unlock(&list->spinlock);

        return 0;
}

/**
 * linklist_is_empty() - check list whether is empty
 *
 * @param list: pointer to linked list
 * @return 1 on empty
 */
int linklist_is_empty(linklist *list)
{
        int ret = 0;

        if (!list)
                return 1;

        if (!list->head)
                return 1;

        pthread_spin_lock(&list->spinlock);

        if (list->element_count == 0)
                ret = 1;

        pthread_spin_unlock(&list->spinlock);

        return ret;
}
