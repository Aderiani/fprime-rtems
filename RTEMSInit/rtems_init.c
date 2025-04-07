#include "rtems_config.h"
#include <rtems.h>
#include <stdio.h>

// // Required function prototype for F Prime main
// extern int fprime_main(int argc, char* argv[]);

// // RTEMS Init task
// rtems_task Init(rtems_task_argument ignored)
// {
//     printf("RTEMS F Prime GR740 Application Starting\n");
    
//     // Call F Prime main function with default arguments
//     char *argv[] = {"fprime", NULL};
//     int result = fprime_main(1, argv);
    
//     printf("F Prime application exited with code: %d\n", result);
//     rtems_task_delete(RTEMS_SELF); // Clean up the task
// }

// Define the Init task configuration
#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)
#define CONFIGURE_INIT_TASK_PRIORITY 10
#define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | RTEMS_TIMESLICE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT