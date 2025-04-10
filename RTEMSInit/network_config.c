
#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE // For compatibility with older versions of RTEMS
#endif


// In network_config.c or a new file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtems/rtems_bsdnet.h>
#include <grlib/network_interface_add.h>
#include <grlib/greth.h>
#include <grlib/ambapp_ids.h>
#include <grlib/ambapp_bus.h>


struct ethernet_config interface_configs[] = {
    // IP address 192.168.0.67 won't conflict with GRMON/EDCL (192.168.0.24)
    { "192.168.0.67", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x79}},
    { NULL, NULL, {0,0,0,0,0,0}}
};






#ifdef ENABLE_NETWORK_SMC_LEON3
struct rtems_bsdnet_ifconfig smcconfig;
#endif

/* This is called from system_init() in rtems-ttcp.c */
void system_init2(void) {
    #ifdef CONFIGURE_DRIVER_PCI_GR_RASTA_IO
    /* Add any PCI-specific initialization here, if needed */
    #endif

    /* Remaining initialization from config_leon3_drvmgr.c would go here */
    printf("Initialized system drivers\n");
}

void system_init(void) {
    /* Initialize driver manager */
    printf("Initializing driver manager\n");
    if (drvmgr_init() != 0) {
        printf("Driver manager failed to initialize\n");
        exit(-1);
    }

    /* CPU/SYSTEM specific Init */
    system_init2();

    #ifdef ENABLE_NETWORK
    /* Init network */
    printf("Initializing network\n");
    rtems_bsdnet_initialize_network();
    printf("Initializing network DONE\n\n");
    rtems_bsdnet_show_inet_routes();
    printf("\n");
    rtems_bsdnet_show_if_stats();
    printf("\n\n");
    #endif
}


