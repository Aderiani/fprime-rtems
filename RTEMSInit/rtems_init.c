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

// Declare C++ runtime initialization (no extern "C" needed in C file)
void __cxx_global_var_init(void);
// Declare FPrime entry point as per rtems_config.h
int fprime_main(int argc, char* argv[]);

rtems_task Init(rtems_task_argument arg) {
    // Initialize C++ runtime for static objects
    __cxx_global_var_init();
    
    // Call FPrime main with dummy arguments
    char* dummy_argv[] = { "fprime", NULL };
    fprime_main(1, dummy_argv);
    
    rtems_task_delete(RTEMS_SELF);
}

// Include the actual configuration
#include <rtems/confdefs.h>