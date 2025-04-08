#ifndef _RTEMS_NETWORKCONFIG_H_
#define _RTEMS_NETWORKCONFIG_H_

#include <grlib/ambapp_bus_grlib.h>
#include <grlib/ambapp_bus.h>
#include <grlib/ambapp_ids.h>
#include <amba.h>

/* GRPCI2 driver configuration (example) */
struct drvmgr_key grlib_drv_res_grpci2_0[] = {
    DRVMGR_KEY_EMPTY
};

/* GRETH driver configuration (optional, adjust as needed) */
struct drvmgr_key grlib_drv_res_greth0[] = {
    DRVMGR_KEY_EMPTY
};

/* Driver resources */
struct drvmgr_bus_res grlib_drv_resources = {
    .next = NULL,
    .resource = {
        {DRIVER_AMBAPP_GAISLER_GRPCI2_ID, 0, &grlib_drv_res_grpci2_0[0]},
        {DRIVER_AMBAPP_GAISLER_GRETH_ID, 0, &grlib_drv_res_greth0[0]}, /* Enable GRETH[0] */
        DRVMGR_RES_EMPTY
    }
};

// struct grlib_config grlib_bus_config = {
//     &ambapp_plb,          /* AMBAPP bus setup */
//     &grlib_drv_resources, /* Driver configuration */
// };

// /* System initialization */
// void system_init2(void) {
//     ambapp_grlib_root_register(&grlib_bus_config); /* Register GRLIB root bus */
// }

// // Network interface configuration
// struct rtems_bsdnet_ifconfig greth_config = {
//     "gr0",                      // Interface name
//     (void*)RTEMS_BSP_NETWORK_DRIVER_ATTACH,  // Attach function
//     NULL,                       // Next interface
//     "192.168.1.10",             // IP address (static)
//     "255.255.255.0",            // Netmask
//     NULL,                       // Hardware address
//     0,                          // Ignore broadcast
//     0,                          // MTU
//     0,                          // RBUF count
//     0,                          // XBUF count
//     0,                          // Driver control
//     NULL                        // Additional driver parameters
// };

// Main network configuration
// struct rtems_bsdnet_config rtems_bsdnet_config = {
//     &greth_config,              // Network interfaces
//     NULL,                       // BOOTP/DHCP
//     64,                         // Network task priority
//     128*1024,                   // Mbuf capacity
//     256*1024,                   // Mbuf cluster capacity
//     "fprime-gr740",             // Hostname
//     "local",                    // Domain name
//     "192.168.1.1",              // Gateway
//     NULL,                       // Log host
//     {NULL},                     // Name servers
//     {NULL},                     // NTP servers
//     0,                          // sb_efficiency
//     0,                          // udp_tx_buf_size
//     0,                          // udp_rx_buf_size
//     0,                          // tcp_tx_buf_size
//     0                           // tcp_rx_buf_size
// };




#endif