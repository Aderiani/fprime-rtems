/*
 * @file network_init.c
 * @brief Network initialization for RTEMS GR740 F' Application
 */

 #include <rtems.h>
 #include <rtems/rtems_bsdnet.h>
 #include <rtems/rtems/tasks.h>
 #include <net/if.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 /* Network configuration structure */
 struct rtems_bsdnet_config rtems_bsdnet_config = {
     .network_task_priority = 100,
     .mbuf_bytecount = 65536,
     .mbuf_cluster_bytecount = 131072,
     .hostname = "gr740-fprime",
     .domainname = "",
     .gateway = NULL,
     .log_host = NULL,
     .name_server = {NULL},
     .ntp_server = {NULL},
     .ifconfig = NULL,
     .bootp = NULL,
 };
 
 /* Network interface configuration */
 static struct rtems_bsdnet_ifconfig netdriver_config = {
     .name = "eth0",
     .attach = NULL,  // Will be set by driver initialization
     .ip_address = NULL,  // Can be set statically or via DHCP
     .ip_netmask = NULL,
     .hardware_address = NULL,
     .ignore_broadcast = 0,
     .mtu = 1500,
     .next = NULL
 };
 
 /* Network configuration options */
 typedef struct {
     int use_dhcp;
     const char* static_ip;
     const char* netmask;
     const char* gateway;
 } NetworkConfig;
 
 /* Initialize network with flexible configuration */
 int initialize_network(NetworkConfig* config) {
     if (!config) {
         printf("Network configuration is NULL\n");
         return -1;
     }
 
     /* Configure static IP or DHCP */
     if (!config->use_dhcp && config->static_ip) {
         netdriver_config.ip_address = (char*)config->static_ip;
         netdriver_config.ip_netmask = (char*)(config->netmask ? config->netmask : "255.255.255.0");
         
         if (config->gateway) {
             rtems_bsdnet_config.gateway = (char*)config->gateway;
         }
         
         rtems_bsdnet_config.bootp = NULL;  // Disable DHCP
     } else {
         /* Use DHCP */
         rtems_bsdnet_config.bootp = rtems_bsdnet_do_dhcp;
         netdriver_config.ip_address = NULL;
         netdriver_config.ip_netmask = NULL;
     }
 
     /* Set network interface configuration */
     rtems_bsdnet_config.ifconfig = &netdriver_config;
 
     /* Initialize network stack */
     printf("Initializing network %s\n", 
            config->use_dhcp ? "(DHCP)" : "(Static IP)");
     
     int result = rtems_bsdnet_initialize_network();
     
     if (result != 0) {
         printf("Network initialization failed\n");
         return -1;
     }
 
     /* Optional: Print network configuration */
     if (!config->use_dhcp && config->static_ip) {
         printf("Network Configured:\n");
         printf("  IP Address: %s\n", config->static_ip);
         printf("  Netmask:    %s\n", 
                config->netmask ? config->netmask : "255.255.255.0");
         if (config->gateway) {
             printf("  Gateway:    %s\n", config->gateway);
         }
     }
 
     return 0;
 }
 
 /* Helper function to create network configuration */
 NetworkConfig* create_network_config(
     int use_dhcp, 
     const char* static_ip, 
     const char* netmask, 
     const char* gateway
 ) {
     NetworkConfig* config = malloc(sizeof(NetworkConfig));
     if (!config) {
         return NULL;
     }
 
     config->use_dhcp = use_dhcp;
     config->static_ip = static_ip;
     config->netmask = netmask;
     config->gateway = gateway;
 
     return config;
 }
 
 /* Cleanup network configuration */
 void cleanup_network_config(NetworkConfig* config) {
     if (config) {
         free(config);
     }
 }