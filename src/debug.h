#ifndef MYCRAFT_DEMO_DEBUG_H
#define MYCRAFT_DEMO_DEBUG_H

#define DEBUG_PRINTF

#define pr_info(...)            do {                                            \
                                        fprintf(stdout, __VA_ARGS__);           \
                                        fflush(stdout);                         \
                                } while (0)

#define pr_info_func(...)       do {                                            \
                                        fprintf(stdout, "%s(): ", __func__);    \
                                        pr_info(__VA_ARGS__);                   \
                                } while (0)

#ifdef DEBUG_PRINTF
#define pr_debug(...)           do {                                            \
                                        fprintf(stdout, __VA_ARGS__);           \
                                        fflush(stdout);                         \
                                } while (0)

#define pr_debug_func(...)      do {                                            \
                                        fprintf(stdout, "%s(): ", __func__);    \
                                        pr_debug(__VA_ARGS__);                  \
                                } while (0)
#else
#define pr_debug(...)           do { } while (0)
#define pr_dbueg_func(...)      do { } while (0)
#endif

#define pr_err(...)             do {                                            \
                                        fprintf(stderr, __VA_ARGS__);           \
                                        fflush(stderr);                         \
                                } while (0)

#define pr_err_func(...)        do {                                            \
                                        fprintf(stderr, "%s(): ", __func__);    \
                                        pr_err(__VA_ARGS__);                    \
                                } while (0)

#define pr_err_alloc()          pr_err_func("failed to allocate memory\n")

#endif //MYCRAFT_DEMO_DEBUG_H
