#ifndef MYCRAFT_DEMO_DEBUG_H
#define MYCRAFT_DEMO_DEBUG_H

#include <stdio.h>
#include <stdint.h>

#define PRINT_INFO_BIT                  (1U << 0)
#define PRINT_ERROR_BIT                 (1U << 1)
#define PRINT_DEBUG_BIT                 (1U << 2)

extern uint32_t g_debug_level;

// fflush() may slow down program
extern uint32_t g_debug_sync;

#define pr_info(...)            do {                                            \
                                        if (g_debug_level & PRINT_INFO_BIT) {   \
                                                fprintf(stdout, __VA_ARGS__);   \
                                                                                \
                                                if (g_debug_sync)               \
                                                        fflush(stdout);         \
                                        }                                       \
                                } while (0)

#define pr_info_func(...)       do {                                            \
                                        fprintf(stdout, "%s(): ", __func__);    \
                                        pr_info(__VA_ARGS__);                   \
                                } while (0)

#define pr_debug(...)           do {                                            \
                                        if (g_debug_level & PRINT_DEBUG_BIT) {  \
                                                fprintf(stdout, __VA_ARGS__);   \
                                                                                \
                                                if (g_debug_sync)               \
                                                        fflush(stdout);         \
                                        }                                       \
                                } while (0)

#define pr_debug_func(...)      do {                                            \
                                        fprintf(stdout, "%s(): ", __func__);    \
                                        pr_debug(__VA_ARGS__);                  \
                                } while (0)

#define pr_err(...)             do {                                            \
                                        if (g_debug_level & PRINT_ERROR_BIT) {  \
                                                fprintf(stderr, __VA_ARGS__);   \
                                                                                \
                                                if (g_debug_sync)               \
                                                        fflush(stderr);         \
                                        }                                       \
                                } while (0)

#define pr_err_func(...)        do {                                            \
                                        fprintf(stderr, "%s(): ", __func__);    \
                                        pr_err(__VA_ARGS__);                    \
                                } while (0)

#define pr_err_alloc()          pr_err_func("failed to allocate memory\n")
#define pr_err_fopen(fp, err)   pr_err_func("failed to open file %s (err: %d)\n", (fp), (err))

#endif //MYCRAFT_DEMO_DEBUG_H
