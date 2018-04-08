#ifndef MYCRAFT_DEMO_THREAD_H
#define MYCRAFT_DEMO_THREAD_H

#include <pthread.h>
#include <semaphore.h>

pthread_t pthread_create_joinable(void *(*func)(void *), void *arg);
pthread_t pthread_create_detached(void *(*func)(void *), void *arg);

void thread_helper_init(void);
void thread_helper_deinit(void);

#endif //MYCRAFT_DEMO_THREAD_H
