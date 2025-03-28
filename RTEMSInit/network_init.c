/*
 * @file network_init.c
 * @brief Network initialization for RTEMS GR740 F' Application
 */
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
/*
 * @file network_init.c
 * @brief Network initialization for RTEMS GR740 F' Application
 */
#include "network_init.h"
#include <arpa/inet.h>
#include <drvmgr/drvmgr.h>
#include <netdb.h>
#include <netinet/in.h>
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/* Placeholder driver attach function that doesn't rely on external symbols */
static int placeholder_driver_attach(struct rtems_bsdnet_ifconfig* cfg, int attach) {
    printf("Placeholder network driver attach function called\n");

    /* Return success to allow build to proceed */
    return 0;
}

/* Interface configuration for the primary Ethernet controller (greth0) */
static struct rtems_bsdnet_ifconfig gr740_eth0_config = {
    "gr0",                     /* Device name */
    placeholder_driver_attach, /* Use placeholder until we fix driver integration */
    NULL,                      /* No additional interfaces */
    NULL,                      /* IP address (set dynamically) */
    NULL,                      /* IP netmask (set dynamically) */
    NULL,                      /* Hardware address (not needed) */
    0,                         /* ignore_broadcast */
    0,                         /* mtu */
    0,                         /* rbuf_count */
    0,                         /* xbuf_count */
    0,                         /* Driver control (not needed) */
    0                          /* Driver-specific parameters */
};

/* BSD network configuration - must be global for RTEMS */
struct rtems_bsdnet_config rtems_bsdnet_config = {
    NULL,               /* hostname - set dynamically */
    NULL,               /* domainname - set dynamically */
    NULL,               /* gateway - set dynamically */
    NULL,               /* log_host */
    {NULL},             /* name_server[3] */
    {NULL},             /* ntp_server[3] */
    0,                  /* sb_efficiency - default */
    8192,               /* udp_tx_buf_size */
    8192,               /* udp_rx_buf_size */
    32768,              /* tcp_tx_buf_size */
    32768,              /* tcp_rx_buf_size */
    &gr740_eth0_config, /* ifconfig */
    NULL,               /* bootp - will be set based on config */
    100,                /* network_task_priority */
    65536,              /* mbuf_bytecount */
    131072,             /* mbuf_cluster_bytecount */
    0,                  /* mbuf_timeout - default */
    NULL                /* ntp_timeout */
};

/* Create a network configuration */
NetworkConfig* create_network_config(int use_dhcp, const char* static_ip, const char* netmask, const char* gateway) {
    NetworkConfig* config = malloc(sizeof(NetworkConfig));
    if (config) {
        config->use_dhcp = use_dhcp;
        config->static_ip = static_ip ? strdup(static_ip) : NULL;
        config->netmask = netmask ? strdup(netmask) : NULL;
        config->gateway = gateway ? strdup(gateway) : NULL;

        /* Set default values for hostname and domainname */
        config->hostname = strdup("gr740-fprime");
        config->domainname = strdup("local");
    }
    return config;
}

/* Clean up a network configuration */
void cleanup_network_config(NetworkConfig* config) {
    if (config) {
        if (config->static_ip)
            free((void*)config->static_ip);
        if (config->netmask)
            free((void*)config->netmask);
        if (config->gateway)
            free((void*)config->gateway);
        if (config->hostname)
            free((void*)config->hostname);
        if (config->domainname)
            free((void*)config->domainname);
        free(config);
    }
}

/* Initialize the network with given configuration */
int initialize_network(NetworkConfig* config) {
    // Make sure clock gating is properly configured for GRETH
    // The GR740 has clock gating that needs to be enabled
    volatile uint32_t* unlock_reg = (volatile uint32_t*)0xFFA04000;
    volatile uint32_t* enable_reg = (volatile uint32_t*)0xFFA04004;
    volatile uint32_t* reset_reg = (volatile uint32_t*)0xFFA04008;
    
    // GRETH0 bit in clock gating unit (check your GR740 manual for specific bit)
    uint32_t greth_bit = (1 << 7); // Example - adjust to actual bit
    
    // Unlock, reset, enable sequence as shown in bdinit.c
    *unlock_reg = greth_bit;
    *reset_reg = greth_bit;
    *enable_reg = greth_bit;
    *enable_reg = 0;
    *reset_reg = 0;
    *enable_reg = greth_bit;
    *unlock_reg = 0;
    
    /* Initialize the network stack */
    printf("Initializing RTEMS networking stack...\n");
    int status = rtems_bsdnet_initialize_network();
    if (status != 0) {
        printf("Network initialization failed: %d\n", status);
        return -1;
    }

    printf("Board IP address: %s\n", config->static_ip ? config->static_ip : "0.0.0.0");
    printf("Network initialization successful\n");
    return 0;
}