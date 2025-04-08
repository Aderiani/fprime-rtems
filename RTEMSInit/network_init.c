#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE // For compatibility with older versions of RTEMS
#endif



#include "network_init.h"
#include <stdio.h>
#include <rtems/rtems_bsdnet.h>
#include <grlib/network_interface_add.h>

/* Declare the system_init function from config.c */
extern void system_init(void);

int initialize_fprime_network() {
    printf("Initializing RTEMS networking stack...\n");
    
    /* Call the system_init function that's used by rtems-ttcp */
    system_init();
    
    printf("Network initialization complete\n");
    return 0;
}