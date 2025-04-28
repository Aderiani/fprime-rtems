#ifndef RTEMS_CONFIG_H
    #define RTEMS_CONFIG_H


#define LEON3  // Needed for some GRLIB configurations

#define RTEMS_DRVMGR_STARTUP 1

#define CONFIGURE_EXECUTIVE_RAM_SIZE	(150*1024*1024) // x MB
#define CONFIGURE_INIT_TASK_STACK_SIZE (2*1024 * 1024) // 2MB
#define CONFIGURE_MALLOC_STATISTICS
#define CONFIGURE_NETWORK_MBUF_BYTES (512*1024)        // 512k for mbufs
#define CONFIGURE_NETWORK_MBUF_CLUSTER_BYTES (1024*1024)  // 1MB for mbuf clusters


// System resources
#define CONFIGURE_MAXIMUM_TASKS 50
#define CONFIGURE_MAXIMUM_SEMAPHORES 50
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 100
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

// Init task table required
 #define CONFIGURE_RTEMS_INIT_TASKS_TABLE
extern int fprime_main(int argc, char* argv[]); // Declare the LedBlinker entry point
#define CONFIGURE_INIT_TASK_PRIORITY 100
#define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | RTEMS_TIMESLICE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INIT_TASK_ENTRY_POINT   fprime_main  // F Prime’s entry point
#define CONFIGURE_INIT_TASK_NAME          rtems_build_name('F', 'P', 'R', 'M')


// Required for networking
#define CONFIGURE_MAXIMUM_DRIVERS 32

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRETH
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2

#define CONFIGURE_MAXIMUM_NETWORK_INTERFACES 2

// Network task configuration
#define CONFIGURE_NETWORK_TASK_PRIORITY 64


#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRGPIO 


// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_OCCAN
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_B1553BRM
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
// #define CONFIGURE_DRIVER_AMBAPP_MCTRL
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF
// #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI
// #define CONFIGURE_DRIVER_PCI_GR_RASTA_IO
// #define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC
// #define CONFIGURE_DRIVER_PCI_GR_701

// IMPORTANT:: SMP OR AMP? THESE DEFINITIONS DEPEND TOTALLY ON THE STRATEGY
// #define CONFIGURE_SMP_APPLICATION
// #define CONFIGURE_SMP_MAXIMUM_PROCESSORS 4
// #define CONFIGURE_SMP_MAXIMUM_PROCESSORS 1
#define CONFIGURE_MAXIMUM_PROCESSORS 1

#define CONFIGURE_MAXIMUM_POSIX_THREADS 32
#define CONFIGURE_MAXIMUM_POSIX_QUEUES 62
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES 62
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES 32
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES 32



// Required drivers
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
// One of the following three must be defined
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
// #define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
// #define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_MEMORY
#define CONFIGURE_APPLICATION_NEEDS_LIBC


#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART


// Use unlimited objects where possible
#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

// Include IMFS filesystem
#define CONFIGURE_FILESYSTEM_IMFS

// // Define struct bintime if missing (add this part)
// #include <stdint.h>
// #include <time.h>
// #ifndef _STRUCT_BINTIME
// #define _STRUCT_BINTIME
// struct bintime {
//     time_t sec;    /* Seconds */
//     uint64_t frac; /* Fractional part in 2^-64 seconds */
// };
// #endif





#define ENABLE_NETWORK


// Initialization
#define CONFIGURE_INIT



#endif  // RTEMS_CONFIG_H



#include <drvmgr/drvmgr_confdefs.h>
#include <rtems/confdefs.h>
#include <stddef.h>
// #include <grlib/network_interface_add.h>  // This file defines the ethernet_config struct
#include <grlib/ambapp_bus.h>
// test_fprime_style.c
#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>
#include <drvmgr/drvmgr.h>
#include "RTEMSInit/network_init.h"

// Forward declaration for a function that would call the F' initialization
void init_fprime(void);

// Simulated F' main function - equivalent to your fprime_main
int fprime_main(int argc, char* argv[]) {
    printf("Fprime-style main starting...\n");
    
    // Initialize fprime components (simulated)
    init_fprime();
    
    printf("Entering F' main infinite loop\n");
    
    // The critical infinite loop
    unsigned int counter = 0;
    while (1) {
        if (counter % 10 == 0) {
            printf("F' style main heartbeat2: %u\n", counter/10);
        }
        counter++;
        
        // Sleep for a bit
        rtems_task_wake_after(100); // 1 second at 100 ticks/sec
    }
    
    printf("This should never be reached!\n");
    return 0;
}

// Standard main that calls fprime_main 
int main(int argc, char* argv[]) {
    printf("Main function starting\n");
    printf("Calling fprime_main\n");
    
    return fprime_main(argc, argv);
}

// Simulated F' initialization function
void init_fprime(void) {
    printf("Initializing driver manager\n");
    
    // Initialize driver manager
    if (drvmgr_init() != 0) {
        printf("Driver manager initialization failed\n");
        exit(1);
    }
    
    printf("Driver manager initialized\n");
    printf("F' initialization complete\n");
}