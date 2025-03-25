/*
 * @file rtems_init.c
 * @brief RTEMS initialization configuration for GR740 with F' integration
 */

 #include <rtems.h>
 #include <rtems/bspIo.h>
 #include <rtems/rtems_bsdnet.h>
 #include <rtems/rtems/tasks.h>
 #include <net/if.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 // Defines to prevent conflicting configurations
 #undef CONFIGURE_MAXIMUM_TASKS
 #undef CONFIGURE_MINIMUM_TASK_STACK_SIZE
 #undef CONFIGURE_INIT
 
 // Network configuration
 struct rtems_bsdnet_config rtems_bsdnet_config = {
     .network_task_priority = 100,
     .mbuf_bytecount = 65536,
     .mbuf_cluster_bytecount = 131072,
     .hostname = "gr740-fprime",
     .domainname = "",
 };
 
 // Declare external functions
 #if defined(__cplusplus)
 extern "C" {
 #endif
     extern int fprime_main(int argc, char* argv[]);
     void *POSIX_Init(void *arg);
 #if defined(__cplusplus)
 }
 #endif
 
 // Prototype for POSIX_Init
 void *POSIX_Init(void *arg);
 
 // Initialization task
 static rtems_task Init(rtems_task_argument arg) {
     // Logging initialization
     printk("RTEMS initialized for F' on GR740\n");
 
     // Prepare F' arguments
     char* fprime_argv[] = {
         "gr740-fprime",
         "--network", 
         "eth0",
         NULL
     };
     int fprime_argc = sizeof(fprime_argv) / sizeof(fprime_argv[0]) - 1;
     
     // Call F' main function
     int result = fprime_main(fprime_argc, fprime_argv);
     
     // Log result and suspend task
     printk("F' Application Completed with result: %d\n", result);
     rtems_task_suspend(RTEMS_SELF);
 }
 
 // Configuration starts here
 #define CONFIGURE_INIT
 #define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
 #define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER
 #define CONFIGURE_APPLICATION_NEEDS_NETWORKING
 
 // Resource Limits
 #define CONFIGURE_MAXIMUM_PROCESSORS 4
 #define CONFIGURE_MAXIMUM_TASKS 64
 #define CONFIGURE_MAXIMUM_SEMAPHORES 32
 #define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 32
 #define CONFIGURE_MAXIMUM_TIMERS 32
 
 // Initialization Task Configuration
 #define CONFIGURE_RTEMS_INIT_TASKS_TABLE
 #define CONFIGURE_INIT_TASK_STACK_SIZE (128 * 1024)
 #define CONFIGURE_INIT_TASK_PRIORITY 120
 #define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | RTEMS_NO_TIMESLICE)
 
 // Networking Configuration
 #define CONFIGURE_MAXIMUM_NETWORK_TASKS 4
 #define CONFIGURE_NETWORK_TASK_STACK_SIZE (64 * 1024)
 
 // Task Stack Configuration
 #define CONFIGURE_MINIMUM_TASK_STACK_SIZE (16 * 1024)
 #define CONFIGURE_IDLE_TASK_INITIALIZES_APPLICATION
 
 // Driver Configuration
 #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRETH
 #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 
 // Include RTEMS configuration
 #include <rtems/confdefs.h>
 
 // POSIX initialization entry point
 void *POSIX_Init(void *arg) {
     Init(0);
     return NULL;
 }