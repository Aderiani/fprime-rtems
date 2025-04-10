#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE // For compatibility with older versions of RTEMS
#endif

#include "network_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For memset
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <grlib/network_interface_add.h>  // For ethernet_config


int initialize_fprime_network() {
    printf("Initializing RTEMS networking stack for GR740...\n");
    
    // Configure network for greth0
    struct rtems_bsdnet_ifconfig *ifconfig;
    
    // Allocate memory for network interface config
    ifconfig = malloc(sizeof(struct rtems_bsdnet_ifconfig));
    if (!ifconfig) {
        printf("Failed to allocate memory for network interface\n");
        return -1;
    }
    
    // Clear memory
    memset(ifconfig, 0, sizeof(struct rtems_bsdnet_ifconfig));
    
    // Configure interface for greth0
    ifconfig->name = "greth0";
    ifconfig->next = NULL;
    
    // Use first entry from interface_configs for greth0
    ifconfig->ip_address = interface_configs[0].ip_addr;
    ifconfig->ip_netmask = interface_configs[0].ip_netmask;
    
    // GRETH driver will be attached via driver manager
    // This is handled automatically with RTEMS_DRVMGR_STARTUP
    
    // Update rtems_bsdnet_config
    rtems_bsdnet_config.ifconfig = ifconfig;
    
    // Initialize driver manager (which manages greth0)
    printf("Initializing driver manager\n");
    if (drvmgr_init() != 0) {
        printf("Driver manager failed to initialize\n");
        return -1;
    }
    
    // Initialize network stack
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
    
    printf("Network initialization complete - greth0 configured with IP %s\n", 
           interface_configs[0].ip_addr);
    return 0;
}