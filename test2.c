// test_rtems_app.c
#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>


// test_rtems_config.h
#ifndef TEST_RTEMS_CONFIG_H
#define TEST_RTEMS_CONFIG_H

// Basic system configuration
#define LEON3
#define RTEMS_DRVMGR_STARTUP 1

// Memory configuration
#define CONFIGURE_EXECUTIVE_RAM_SIZE (64*1024*1024)  // 64 MB
#define CONFIGURE_INIT_TASK_STACK_SIZE (32*1024)     // 32 KB

// Task configuration
#define CONFIGURE_MAXIMUM_TASKS 10
#define CONFIGURE_MAXIMUM_SEMAPHORES 10
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 10
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 10

// Only use a single processor to start with
#define CONFIGURE_MAXIMUM_PROCESSORS 1

// Initial task configuration
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_PRIORITY 100
#define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | RTEMS_TIMESLICE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

// Required drivers
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRETH

// Initialization
#define CONFIGURE_INIT

#endif // TEST_RTEMS_CONFIG_H


// Additional includes for SMP
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp_bus.h>

// Global variable to track if we should exit
volatile int should_exit = 0;

// Secondary task to run on other processors
rtems_task Secondary_Task(rtems_task_argument arg) {
    unsigned int cpu_num = arg;
    unsigned int counter = 0;
    
    printf("Secondary task %u started on CPU %u\n", cpu_num, rtems_get_current_processor());
    
    while (!should_exit) {
        printf("Secondary task %u alive - count: %u\n", cpu_num, counter++);
        rtems_task_wake_after(rtems_clock_get_ticks_per_second());
    }
    
    printf("Secondary task %u exiting\n", cpu_num);
    rtems_task_delete(RTEMS_SELF);
}

// Main initialization task
rtems_task Init(rtems_task_argument argument) {
    rtems_status_code status;
    rtems_id task_id;
    rtems_name task_name;
    unsigned int counter = 0;
    
    printf("***** GR740 RTEMS SMP Test Application Starting *****\n");
    printf("Running on CPU: %u\n", rtems_get_current_processor());
    
    // Initialize driver manager
    printf("Initializing driver manager...\n");
    if (drvmgr_init() != 0) {
        printf("Driver manager initialization failed\n");
        exit(1);
    }
    
    // Create tasks for other processors if in SMP mode
#if CONFIGURE_MAXIMUM_PROCESSORS > 1
    printf("Creating tasks for other CPUs...\n");
    for (int i = 1; i < CONFIGURE_MAXIMUM_PROCESSORS; i++) {
        task_name = rtems_build_name('S', 'T', '0' + i, ' ');
        status = rtems_task_create(
            task_name,
            100,  // Priority
            16 * 1024,  // Stack size
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &task_id
        );
        
        if (status != RTEMS_SUCCESSFUL) {
            printf("Failed to create task for CPU %d\n", i);
            continue;
        }
        
        status = rtems_task_start(task_id, Secondary_Task, i);
        if (status != RTEMS_SUCCESSFUL) {
            printf("Failed to start task for CPU %d\n", i);
        }
    }
#endif
    
    // Main loop
    printf("Entering main loop...\n");
    while (1) {
        printf("Main task alive on CPU %u - count: %u\n", 
               rtems_get_current_processor(), counter++);
        rtems_task_wake_after(rtems_clock_get_ticks_per_second());
        
        // For testing purposes, we could set an exit condition
        // For example, exit after 100 iterations
        // if (counter >= 100) break;
    }
    
    // Signal secondary tasks to exit
    should_exit = 1;
    
    // Wait for tasks to exit
    printf("Waiting for secondary tasks to exit...\n");
    rtems_task_wake_after(5 * rtems_clock_get_ticks_per_second());
    
    // Should never reach here in normal operation
    printf("Main loop exited (should never happen)\n");
    exit(0);
}