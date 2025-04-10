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

// // In RTEMSInit/network_init.c
// int initialize_fprime_network() {
//     printf("Initializing RTEMS networking stack for GR740...\n");

//     // Initialize driver manager first
//     printf("Initializing driver manager\n");
//     if (drvmgr_init() != 0) {
//         printf("Driver manager failed to initialize\n");
//         return -1;
//     }

//     // Call system_init2() to initialize AMBA bus
//     system_init2();

//     // Configure network interfaces
//     printf("Configuring network interfaces\n");
//     struct rtems_bsdnet_ifconfig *ifconfig;
//     ifconfig = malloc(sizeof(struct rtems_bsdnet_ifconfig));
//     if (!ifconfig) {
//         printf("Failed to allocate memory for network interface\n");
//         return -1;
//     }

//     memset(ifconfig, 0, sizeof(struct rtems_bsdnet_ifconfig));

//     // Configure Ethernet (greth) interface
//     ifconfig->name = "gr_eth1";  // This is the name GRETH typically uses
//     ifconfig->attach = NULL;      // Will be handled by driver manager
//     ifconfig->next = NULL;
//     ifconfig->ip_address = "192.168.0.67";
//     ifconfig->ip_netmask = "255.255.255.0";

//     // Set MAC address from interface_configs
//     memcpy(ifconfig->hardware_address, interface_configs[0].eth_adr, 6);
//     ifconfig->ignore_broadcast = 0;
//     ifconfig->mtu = 1500;
//     ifconfig->rbuf_count = 8;
//     ifconfig->xbuf_count = 4;

//     // Additional GRETH-specific settings if needed
//     // ifconfig->drv_ctrl = &greth_config;

//     // Update rtems_bsdnet_config
//     printf("Setting up network config\n");
//     rtems_bsdnet_config.ifconfig = ifconfig;
//     rtems_bsdnet_config.hostname = "fprime-gr740";
//     rtems_bsdnet_config.domainname = "local";
//     rtems_bsdnet_config.gateway = "192.168.0.1";
//     rtems_bsdnet_config.log_host = NULL;
//     rtems_bsdnet_config.name_server[0] = NULL;
//     rtems_bsdnet_config.ntp_server[0] = NULL;
//     rtems_bsdnet_config.network_task_priority = 100;
//     rtems_bsdnet_config.mbuf_bytecount = 65536;
//     rtems_bsdnet_config.mbuf_cluster_bytecount = 131072;

//     // Initialize network stack
//     printf("Starting network stack...\n");
//     int result = rtems_bsdnet_initialize_network();
//     if (result != 0) {
//         printf("ERROR: Network initialization failed: %d\n", result);
//         return -1;
//     }

//     // Verify interface is up
//     printf("Checking network interfaces...\n");
//     rtems_bsdnet_show_if_stats();

//     return 0;
// }



// Reference to board initialization
extern int board_initialize(void);

int initialize_fprime_network(void) {
    printf("Initializing RTEMS networking stack for GR740...\n");
    
    // Initialize board hardware first
    if (board_initialize() != 0) {
        printf("Board initialization failed\n");
        return -1;
    }
    
    // Now initialize the network stack
    printf("Starting network stack...\n");
    
    // Define ENABLE_NETWORK explicitly here if needed
    #ifndef ENABLE_NETWORK
    #define ENABLE_NETWORK
    #endif
    
    int result = rtems_bsdnet_initialize_network();
    if (result != 0) {
        printf("ERROR: Network initialization failed: %d\n", result);
        return -1;
    }
    
    // Add a significant delay for network to fully initialize
    printf("Waiting for network interfaces to come up...\n");
    rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 5);
    
    printf("Network initialization complete\n");
    rtems_bsdnet_show_inet_routes();
    rtems_bsdnet_show_if_stats();
    
    return 0;
}