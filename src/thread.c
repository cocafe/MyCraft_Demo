#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include <GLFW/glfw3.h>

#include "utils.h"
#include "debug.h"
#include "thread.h"

pthread_attr_t thread_attr_join;
pthread_attr_t thread_attr_detach;

pthread_t pthread_create_joinable(void *(*func)(void *), void *arg)
{
        pthread_t t;
        int ret;

        ret = pthread_create(&t, &thread_attr_join, func, arg);
        if (ret)
                pr_err_func("failed to create thread\n");

        return t;
}

pthread_t pthread_create_detached(void *(*func)(void *), void *arg)
{
        pthread_t t;
        int ret;

        ret = pthread_create(&t, &thread_attr_detach, func, arg);
        if (ret)
                pr_err_func("failed to create thread\n");

        return t;
}

void thread_helper_init(void)
{
        pthread_attr_init(&thread_attr_join);
        pthread_attr_init(&thread_attr_detach);

        pthread_attr_setdetachstate(&thread_attr_join, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setdetachstate(&thread_attr_detach, PTHREAD_CREATE_DETACHED);
}

void thread_helper_deinit(void)
{
        pthread_attr_destroy(&thread_attr_join);
        pthread_attr_destroy(&thread_attr_detach);
}

