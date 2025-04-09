
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

// Function to register and enable greth1
int register_greth_driver(void) {
    printf("Attempting to enable greth1 for network use...\n");
    
    // Try to un-clockgate greth1 by writing to its control register
    volatile unsigned int *greth1_ctrl = (volatile unsigned int *)0xff980000;
    printf("Writing to GRETH1 control register at 0x%p\n", greth1_ctrl);
    
    // Enable the GRETH controller by setting bit 0 (typically the enable bit)
    // We read-modify-write to preserve other bits
    unsigned int val = *greth1_ctrl;
    printf("Current GRETH1 control register value: 0x%08x\n", val);
    
    // Set enable bit and clear reset bit if present
    val |= 0x00000001;  // Enable bit
    *greth1_ctrl = val;
    
    printf("Updated GRETH1 control register to: 0x%08x\n", val);
    
    // Configure a network interface for greth1
    struct rtems_bsdnet_ifconfig *ifconfig = malloc(sizeof(struct rtems_bsdnet_ifconfig));
    if (!ifconfig) {
        printf("Failed to allocate memory for network interface\n");
        return -1;
    }
    
    memset(ifconfig, 0, sizeof(struct rtems_bsdnet_ifconfig));
    
    // Configure the interface
    ifconfig->name = "greth1";
    ifconfig->attach = NULL; // Use driver manager registration
    ifconfig->next = rtems_bsdnet_config.ifconfig;
    ifconfig->ip_address = "192.168.0.67";
    ifconfig->ip_netmask = "255.255.255.0";
    
    // Register with network stack
    printf("Adding greth1 network interface\n");
    network_interface_add(ifconfig);
    
    // Update the network configuration
    rtems_bsdnet_config.ifconfig = ifconfig;
    
    return 0;
}

/* From the sample app - use the same structure */
struct ethernet_config interface_configs[] = {
    { "192.168.0.67", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x79}},
    { "192.168.1.67", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7A}},
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


// GRETH driver configuration for greth1
struct drvmgr_key grlib_drv_res_greth1[] = {
    {"drvIndex", DRVMGR_KT_INT, {(unsigned int)1}}, // Use index 1 for greth1
    {"useDMAArea", DRVMGR_KT_INT, {(unsigned int)1}},
    {"bdDMAAreaSize", DRVMGR_KT_INT, {(unsigned int)(8*1024)}},
    DRVMGR_KEY_EMPTY
};

// Add this to your driver resources in config_leon3_drvmgr.c
struct drvmgr_bus_res grlib_drv_resources = {
    .next = NULL,
    .resource = {
        // Add this line to target greth1 specifically
        {DRIVER_AMBAPP_GAISLER_GRETH_ID, 1, &grlib_drv_res_greth1[0]},
        // Other resources...
        DRVMGR_RES_EMPTY
    }
};
