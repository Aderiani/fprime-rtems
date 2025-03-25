/*
 * @file rtems_init.c
 * @brief RTEMS initialization configuration for GR740 with F' integration
 */
#ifndef IFNAMSIZ
#define IFNAMSIZ 16  // Typical value for interface name size
#endif

 #include <rtems.h>
 #include <rtems/bspIo.h>
 #include <rtems/rtems_bsdnet.h>
 #include <rtems/rtems/tasks.h>
 #include <sys/time.h>
 #include <bsp.h>
 #include <rtems/confdefs.h>
 #include <net/if.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
#include "network_init.h"

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             20
#define CONFIGURE_MAXIMUM_PROCESSORS        4  // GR740 SMP has 4 cores
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_TIMERS            10
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    10
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS   1
#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_MINIMUM_STACK_SIZE        (8 * 1024)  // 8KB minimum stack
#define CONFIGURE_STACK_CHECKER_ENABLED
#define CONFIGURE_INIT_TASK_STACK_SIZE      (16 * 1024) // 16KB for Init task
#define CONFIGURE_MICROSECONDS_PER_TICK     1000        // 1ms tick
#define CONFIGURE_MAXIMUM_DRIVERS           10
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

// SMP-specific configuration
#define CONFIGURE_SCHEDULER_PRIORITY_SMP
#include <rtems/score/schedulersmpimpl.h>

// Networking support
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 2
#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE (64 * 1024)

// Define Init task
rtems_task Init(rtems_task_argument argument) {
    NetworkConfig* config = create_network_config(
        "gr740_host", "example.com", "192.168.1.1", "192.168.1.10", "255.255.255.0"
    );

    if (initialize_network(config) != 0) {
        printf("Network initialization failed\n");
        exit(1);
    }

    printf("RTEMS and network initialized. Starting F Prime...\n");

    int fprime_argc = sizeof(fprime_argv) / sizeof(fprime_argv[0]) - 1;
    
    // Call F' main function
    int result = fprime_main(fprime_argc, fprime_argv);

    cleanup_network_config(config);
    exit(0);
}

#include <rtems/confdefs.h>


 


    

