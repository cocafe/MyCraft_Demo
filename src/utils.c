#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>

#include "debug.h"
#include "utils.h"

int ivec3_equal(const ivec3 a, const ivec3 b)
{
        for (int i = 0; i < 3; ++i) {
                if (a[i] != b[i])
                        return 0;
        }

        return 1;
}

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
        list->count_seqlist = count;
        list->count_utilized = 0;

        return 0;
}

int seqlist_deinit(seqlist *list)
{
        if (!list)
                return -EINVAL;

        if (!list->data)
                return -ENODATA;

        free(list->data);
        memzero(list, sizeof(seqlist));

        return 0;
}

int seqlist_expand(seqlist *list, size_t count)
{
        void *new_data;
        size_t new_count;

        if (!list)
                return -EINVAL;

        new_count = list->count_seqlist + count;

        /*
         * We do not use realloc() here, realloc() does not guarantee
         * re-allocated memory block is clear, and we do some bytes copies
         * in seqlist_append()
         */
        new_data = calloc(new_count, list->element_size * new_count);
        if (!new_data) {
                pr_err_alloc();
                return -ENOMEM;
        }

        memcpy(new_data, list->data, list->element_size * list->count_utilized);
        free(list->data);

        list->data = new_data;
        list->count_seqlist = new_count;

        return 0;
}

int seqlist_shrink(seqlist *list)
{
        void *new_data;

        if (!list)
                return -EINVAL;

        new_data = realloc(list->data, list->element_size * list->count_utilized);
        if (!new_data) {
                pr_err_alloc();
                return -ENOMEM;
        }

        list->data = new_data;
        list->count_seqlist = list->count_utilized;

        return 0;
}

int seqlist_append(seqlist *list, void *element)
{
        uint8_t *t;
        size_t i;
        size_t element_size;
        int ret;

        if (!list || !element)
                return -EINVAL;

        if (list->count_utilized >= list->count_seqlist) {
                ret = seqlist_expand(list, SEQLIST_EXPAND_COUNT);
                if (ret) {
                        pr_err_func("failed to expand sequence list\n");
                        return ret;
                }
        }

        // XXX: Memory block pointer is changed after seqlist_expand()
        t = (uint8_t *)list->data;
        i = list->count_utilized;
        element_size = list->element_size;

        memcpy(&t[(element_size / sizeof(uint8_t)) * i], element, element_size);

        list->count_utilized++;

        return 0;
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
        n->mark_delete = 0;

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

        return 0;
}

int linklist_deinit(linklist *list)
{
        linklist_node *curr, *next;

        if (!list)
                return -EINVAL;

        if (!list->head)
                return -ENODATA;

        curr = list->head;
        while (curr) {
                next = curr->next;

                linklist_node_deinit(curr);
                linklist_node_free(&curr);

                curr = next;
        }

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
        void *data;
        int ret;

        if (!list || !element)
                return NULL;

        if (list->head == NULL && list->element_count != 0) {
                pr_err_func("program internal error\n");
                return NULL;
        }

        ret = linklist_node_alloc(&node);
        if (ret)
                return NULL;

        ret = linklist_node_init(node, NULL, NULL,
                                 element, list->element_size);
        if (ret)
                goto node_free;

        data = node->data;

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
        list->element_count++;

out:
        return data;

node_free:
        linklist_node_free(&node);
        return NULL;
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

        if (list->head == node) {
                next = list->head->next;

                __linklist_node_delete(list, &list->head);

                list->head = next;
                if (list->head != NULL)
                        list->head->prev = NULL;

                return 0;
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

        while (list->head && list->head->mark_delete) {
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

                if (curr->mark_delete) {
                        prev->next = next;
                        if (next != NULL)
                                next->prev = prev;

                        __linklist_node_delete(list, &curr);

                        curr = prev;
                        continue;
                }

                curr = next;
        }

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
        if (!list)
                return 1;

        if (!list->head)
                return 1;

        if (list->element_count == 0)
                return 1;

        return 0;
}
