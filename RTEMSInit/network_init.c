/*
 * @file network_init.c
 * @brief Network initialization for RTEMS GR740 F' Application
 */
#ifndef IFNAMSIZ
#define IFNAMSIZ 16  // Typical value for interface name size
#endif

#include <rtems/rtems_bsdnet.h>
#include "network_init.h"
#include <stdlib.h>  // For malloc/free
#include <string.h>  // For strcpy


struct rtems_bsdnet_ifconfig gr740_ifconfig = {
    .name = "gr0",  // Adjust based on your GR740 network driver (e.g., GRETH)
    .ip_address = NULL,  // Set dynamically below
    .ip_netmask = NULL,  // Set dynamically below
    .hardware_address = NULL,
    .ignore_broadcast = 0,
    .mtu = 0,
    .rbuf_count = 0,
    .xbuf_count = 0,
    .drv_ctrl = NULL
};

int initialize_network(NetworkConfig* config) {
    if (config == NULL) return -1;

    // Set interface-specific IP and netmask
    gr740_ifconfig.ip_address = config->ip_address;
    gr740_ifconfig.ip_netmask = config->ip_netmask;

    // Populate rtems_bsdnet_config
    rtems_bsdnet_config.hostname = config->hostname;
    rtems_bsdnet_config.domainname = config->domainname;
    rtems_bsdnet_config.gateway = config->gateway;
    rtems_bsdnet_config.ifconfig = &gr740_ifconfig;

    // Use BOOTP/DHCP if no static IP is provided
    if (config->ip_address == NULL || config->ip_netmask == NULL) {
        rtems_bsdnet_config.bootp = rtems_bsdnet_do_bootp;  // Or DHCP if available
    } else {
        rtems_bsdnet_config.bootp = NULL;  // Static IP configuration
    }

    // Initialize the network
    if (rtems_bsdnet_initialize_network() != 0) {
        return -1;
    }
    return 0;
}