
// /*
//  * @file network_init.c
//  * @brief Network initialization for RTEMS GR740 F' Application
//  */
// #ifndef IFNAMSIZ
// #define IFNAMSIZ 16
// #endif

// #include "network_init.h"
// #include <arpa/inet.h>
// #include <drvmgr/drvmgr.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <rtems.h>
// #include <rtems/rtems_bsdnet.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/socket.h>

// // Ensure GRETH driver attach function is available
// extern int rtems_greth_driver_attach(struct rtems_bsdnet_ifconfig *config, int attaching);

// // Aligned string duplication for SPARC alignment requirements
// char* aligned_strdup(const char* src) {
//     char* aligned;
//     if (posix_memalign((void**)&aligned, 4, strlen(src) + 1) != 0) return NULL;
//     strcpy(aligned, src);
//     return aligned;
// }

// // Optional loopback interface (uncomment #define RTEMS_USE_LOOPBACK to enable)
// #ifdef RTEMS_USE_LOOPBACK
// void rtems_bsdnet_loopattach(struct rtems_bsdnet_ifconfig *config, int attaching) {
//     // Minimal loopback attach function (placeholder)
//     printf("Loopback interface attached: %s\n", config->name);
// }
// static struct rtems_bsdnet_ifconfig loopback_config = {
//     "lo0",              // name
//     rtems_bsdnet_loopattach,  // attach function
//     NULL,               // next interface (set dynamically)
//     "127.0.0.1",        // IP address
//     "255.0.0.0",        // netmask
//     NULL,               // hardware (driver-specific)
//     0, 0, 0, 0,         // flags and buffer counts
//     NULL                // driver-specific parameters
// };
// #endif

// // GRETH interface configuration for GR740
// static unsigned char greth_mac[6] = {0x00, 0x80, 0x7F, 0x22, 0x61, 0x79};
// static struct rtems_bsdnet_ifconfig gr740_eth_config = {
//     "gr1",
//     rtems_greth_driver_attach,
//     NULL,
//     NULL,
//     NULL,
//     greth_mac,  // hardware points to MAC address
//     0, 0, 0, 0,
//     NULL
// };

// // Consolidated network configuration
// struct rtems_bsdnet_config rtems_bsdnet_config = {
//     #ifdef RTEMS_USE_LOOPBACK
//         &loopback_config,  // First interface (loopback)
//     #else
//         &gr740_eth_config, // First interface (GRETH)
//     #endif
//     NULL,               // bootp (set dynamically)
//     100,                // network task priority
//     65536,              // mbuf_bytecount
//     131072,             // mbuf_cluster_bytecount
//     0,                  // sb_efficiency (default)
//     8192,               // udp_tx_buf_size
//     8192,               // udp_rx_buf_size
//     32768,              // tcp_tx_buf_size
//     32768,              // tcp_rx_buf_size
//     NULL,               // hostname (set dynamically)
//     NULL,               // domainname (set dynamically)
//     NULL,               // gateway (set dynamically)
//     NULL,               // log_host
//     {NULL},             // name_server[3]
//     {NULL},             // ntp_server[3]
//     0,                  // mbuf_timeout (default)
//     NULL                // ntp_timeout
// };

// /* Create a network configuration */
// NetworkConfig* create_network_config(int use_dhcp, const char* static_ip, const char* netmask, const char* gateway) {
//     NetworkConfig* config = malloc(sizeof(NetworkConfig));
//     if (config) {
//         config->use_dhcp = use_dhcp;
//         config->static_ip = static_ip ? aligned_strdup(static_ip) : NULL;
//         config->netmask = netmask ? aligned_strdup(netmask) : NULL;
//         config->gateway = gateway ? aligned_strdup(gateway) : NULL;
//         config->hostname = aligned_strdup("gr740-fprime");
//         config->domainname = aligned_strdup("local");
//     }
//     return config;
// }

// /* Clean up a network configuration */
// void cleanup_network_config(NetworkConfig* config) {
//     if (config) {
//         free((void*)config->static_ip);
//         free((void*)config->netmask);
//         free((void*)config->gateway);
//         free((void*)config->hostname);
//         free((void*)config->domainname);
//         free(config);
//     }
// }

// /* Initialize the network with given configuration */
// int initialize_network(NetworkConfig* config) {
//     printf("Initializing RTEMS networking stack...\n");
//     printf("Initializing RTEMS networking stack...\n");
//     printf("gr740_eth_config address: %p\n", (void*)&gr740_eth_config);
//     printf("gr740_eth_config.name: %p\n", (void*)gr740_eth_config.name);
//     printf("gr740_eth_config.ip_address: %p\n", (void*)gr740_eth_config.ip_address);

//     // Initialize driver manager if not already done by BSP
// #ifndef RTEMS_DRVMGR_STARTUP
//     if (drvmgr_init() != 0) {
//         printf("Driver manager failed to initialize\n");
//         return -1;
//     }
//     printf("Driver manager initialized\n");
// #endif

//     // Configure the interface
//     if (!config->use_dhcp) {
//         gr740_eth_config.ip_address = (char*)config->static_ip;
//         gr740_eth_config.ip_netmask = (char*)config->netmask;
//         rtems_bsdnet_config.gateway = (char*)config->gateway;
//         rtems_bsdnet_config.hostname = (char*)config->hostname;
//         rtems_bsdnet_config.domainname = (char*)config->domainname;
//         rtems_bsdnet_config.name_server[0] = "192.168.0.1"; // Example DNS
//         rtems_bsdnet_config.ntp_server[0] = "192.168.0.1"; // Example NTP
//         rtems_bsdnet_config.bootp = NULL;
//     } else {
//         gr740_eth_config.ip_address = NULL;
//         gr740_eth_config.ip_netmask = NULL;
//         rtems_bsdnet_config.gateway = NULL;
//         rtems_bsdnet_config.hostname = NULL;
//         rtems_bsdnet_config.domainname = NULL;
//         rtems_bsdnet_config.name_server[0] = NULL;
//         rtems_bsdnet_config.ntp_server[0] = NULL;
//         rtems_bsdnet_config.bootp = rtems_bsdnet_do_bootp;
//     }

//     // Link interfaces (GRETH follows loopback if enabled)
// #ifdef RTEMS_USE_LOOPBACK
//     loopback_config.next = &gr740_eth_config;
// #endif

//     // Initialize the network stack
//     int status = rtems_bsdnet_initialize_network();
//     if (status != 0) {
//         printf("Network initialization failed: %d\n", status);
//         return -1;
//     }

//     printf("Network initialized with IP: %s\n", 
//            config->use_dhcp ? "DHCP" : config->static_ip);
//     return 0;
// }

// /* Optional: System initialization wrapper (replacing config.c role) */
// void network_init(void) {
//     NetworkConfig* config = create_network_config(0, "192.168.0.100", "255.255.255.0", "192.168.0.1");
//     if (initialize_network(config) != 0) {
//         printf("System initialization failed\n");
//         exit(-1);
//     }
//     cleanup_network_config(config);
//     printf("System initialization complete\n");
//     rtems_bsdnet_show_if_stats();
// }
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif


#include "network_init.h"
#include "network_config.h"
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/error.h>
#include <errno.h>
#include <rtems/dhcp.h>
#include <string.h>
// In RTEMSInit/network_init.c
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp_bus.h>
#include <grlib/greth.h>
#include <grlib/network_interface_add.h>
#include <grlib/ambapp_ids.h>
#include <stdlib.h>

// Declare network driver attach function (this might need to be provided by your BSP)
extern int rtems_greth_driver_attach(struct rtems_bsdnet_ifconfig *config);
extern void system_init2(void);
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp_ids.h> // For DRIVER_AMBAPP_GAISLER_GRETH_ID

struct drvmgr_dev *find_greth_device(int unit) {
    struct find_arg {
        int target_unit;
        int current_unit;
        struct drvmgr_dev *dev;
    } arg = {unit, 0, NULL};

    int check_greth(struct drvmgr_dev *dev, void *user_arg) {
        struct find_arg *a = (struct find_arg *)user_arg;
        if (dev->bus_type == DRVMGR_BUS_TYPE_AMBAPP && 
            dev->drv && 
            dev->drv->drv_id == DRIVER_AMBAPP_GAISLER_GRETH_ID) {
            if (a->current_unit == a->target_unit) {
                a->dev = dev;
                return 1; // Stop iteration
            }
            a->current_unit++;
        }
        return 0;
    }

    drvmgr_for_each_dev(check_greth, &arg, DRVMGR_FED_DF); // Depth-first search
    return arg.dev;
}

int initialize_fprime_network() {
    // Use default config if NULL is passed
    // if (config == NULL) {
    //     printf("config is null\n");
    //     return -1;
    // }
    system_init2(); /* Initialize drivers */
    printf("Drivers initialized\n");

    // Detect GRETH device
    int greth_unit = find_greth_device();
    if (greth_unit < 0) {
        printf("ERROR: No GRETH network device detected\n");
        return -1;
    }

    // Verbose logging
    printf("Network Initialization Start\n");
    // printf("DHCP: %s\n", config->use_dhcp ? "Enabled" : "Disabled");
    // printf("Static IP: %s\n", config->static_ip);
    // printf("Netmask: %s\n", config->netmask);
    // printf("Gateway: %s\n", config->gateway);
    // printf("Hostname: %s\n", config->hostname);
    printf("GRETH Unit: %d\n", greth_unit);

    // Construct interface name dynamically
    char if_name[16];
    snprintf(if_name, sizeof(if_name), "gr%d", greth_unit);

    // Network interface configuration
    struct rtems_bsdnet_ifconfig* ifconfig = malloc(sizeof(struct rtems_bsdnet_ifconfig));
    if (!ifconfig) {
        printf("Failed to allocate network interface configuration\n");
        return -1;
    }

    extern void greth_register_drv(void);
    greth_register_drv(); // Registers the GRETH driver
    printf("GRETH driver registered\n");

    memset(ifconfig, 0, sizeof(struct rtems_bsdnet_ifconfig));
    ifconfig->name = if_name;
    ifconfig->attach = rtems_greth_driver_attach;  // Use the actual driver attach function
    ifconfig->ip_address = (char*)"192.168.0.67";
    ifconfig->ip_netmask = (char*)"255.255.255.0";

    struct rtems_bsdnet_config rtems_bsdnet_config = {
        .ifconfig = "greth0 192.168.0.67 netmask 255.255.255.0 up",
        .gateway = "192.168.0.1",
        .bootp = NULL,
        .hostname = "fprime-gr740",
    };

    // Initialize network
    int status = rtems_bsdnet_initialize_network();
    if (status != 0) {
        printf("Network initialization failed with status: %d\n", status);
        printf("Errno: %d (%s)\n", errno, strerror(errno));
        free(ifconfig);
        return -1;
    }

    printf("Network initialization successful\n");
    return 0;
}