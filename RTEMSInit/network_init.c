#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE // For compatibility with older versions of RTEMS
#endif

#include "network_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <grlib/network_interface_add.h>

extern void system_init(void);

// Added retry capability for network initialization
int initialize_fprime_network() {
    int retry_count = 0;
    int max_retries = 3;
    
    printf("Initializing RTEMS networking stack...\n");
    
    // Call the existing system initialization function
    system_init();
    
    // Verify network is active by checking interfaces
    struct rtems_bsdnet_ifconfig *ifp;
    retry_count = 0;
    while (retry_count < max_retries) {
        if (rtems_bsdnet_config.ifconfig != NULL) {
            printf("Network interfaces configured successfully\n");
            break;
        }
        
        printf("Waiting for network interfaces to be ready (%d/%d)...\n", 
               retry_count+1, max_retries);
        rtems_task_wake_after(rtems_clock_get_ticks_per_second());
        retry_count++;
    }
    
    if (retry_count >= max_retries && rtems_bsdnet_config.ifconfig == NULL) {
        printf("Warning: Network interfaces not ready after retries\n");
    }
    
    printf("Network initialization complete (status: %s)\n", 
           (rtems_bsdnet_config.ifconfig != NULL) ? "SUCCESS" : "DEGRADED");
    return 0;
}