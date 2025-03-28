// test_rtems_cv.c
#include <rtems.h>
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
volatile int flag = 0;

void *thread_func(void *arg) {
    printf("Thread started\n");
    
    pthread_mutex_lock(&mutex);
    while (flag == 0) {
        printf("Thread waiting on condition\n");
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Thread woken up, flag = %d\n", flag);
    pthread_mutex_unlock(&mutex);
    
    printf("Thread exiting\n");
    return NULL;
}

rtems_task Init(rtems_task_argument ignored) {
    printf("RTEMS Init task started\n");
    
    pthread_t thread;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_create(&thread, &attr, thread_func, NULL);
    
    rtems_task_wake_after(rtems_clock_get_ticks_per_second());
    printf("Main task setting flag and signaling\n");
    
    pthread_mutex_lock(&mutex);
    flag = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    pthread_join(thread, NULL);
    printf("Thread joined successfully\n");
    
    printf("Test completed successfully\n");
    rtems_shutdown_executive(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS 4
#define CONFIGURE_MAXIMUM_POSIX_THREADS 4
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES 4
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES 4
#define CONFIGURE_INIT
#include <rtems/confdefs.h>