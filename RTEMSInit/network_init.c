#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE  // For compatibility with older versions of RTEMS
#endif

#include "network_init.h"
#include <errno.h>
#include <grlib/network_interface_add.h>  // For ethernet_config
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For memset


// Reference to board initialization
extern int board_initialize(void);
int initialize_fprime_network(void) {
    // printf("Initializing RTEMS networking for GR740...\n");

#ifdef RTEMS_DRVMGR_STARTUP
    printf("Using RTEMS_DRVMGR_STARTUP mode\n");
#else
    printf("WARNING: RTEMS_DRVMGR_STARTUP not defined\n");
#endif

    // Initialize board hardware first
    if (board_initialize() != 0) {
        printf("Board initialization failed\n");
        return -1;
    }

    // Initialize network directly using rtems_bsdnet_initialize_network
    // printf("Starting network stack with explicit initialization...\n");



    // printf("Checking if GRETH driver is available...\n");
    struct drvmgr_drv *drv = drvmgr_drv_by_name("GRETH_DRV");
    if (drv == NULL) {
        printf("ERROR: GRETH driver not available in BSP\n");
        // Try forcing registration if not available
        printf("Attempting to initialize networking without GRETH driver...\n");
    } else {
        // printf("GRETH driver found, initializing network interfaces\n");
    }


    int result = rtems_bsdnet_initialize_network();
    if (result != 0) {
        printf("ERROR: Network initialization failed: %d (errno=%d)\n", result, errno);
        return -1;
    }

    // Add significant delay and display network configuration
    // printf("Network initialized, waiting for interfaces...\n");
    rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 2);

    // Show network interfaces and routes
    // printf("NETWORK CONFIGURATION:\n");
    // rtems_bsdnet_show_inet_routes();
    // rtems_bsdnet_show_if_stats();
    // rtems_bsdnet_show_mbuf_stats();

    return 0;
}