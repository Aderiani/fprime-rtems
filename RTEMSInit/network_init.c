#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE // For compatibility with older versions of RTEMS
#endif
// In network_init.c
#include "network_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>

// External functions
extern void system_init(void);
int initialize_fprime_network() {
    printf("Initializing RTEMS networking stack...\n");

    // Initialize driver manager and system
    system_init();
    
    // Initialize the RTEMS network stack
    printf("Starting network stack...\n");
    int result = rtems_bsdnet_initialize_network();
    if (result != 0) {
        printf("ERROR: Network initialization failed: %d\n", result);
        return -1;
    }
    
    // Display network information
    printf("Network routes:\n");
    rtems_bsdnet_show_inet_routes();
    printf("\nNetwork interface statistics:\n");
    rtems_bsdnet_show_if_stats();
    
    printf("Network initialization complete\n");
    return 0;
}