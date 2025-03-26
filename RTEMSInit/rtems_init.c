/*
 * Minimal RTEMS initialization for F Prime GR740 application
 */

/* Required RTEMS headers */
#include <rtems.h>
#include <bsp.h>
#include <sys/time.h>

/* Standard C headers */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Include RTEMS configuration */
#include "rtems_config.h"

/* Network configuration structure */
typedef struct {
    int use_dhcp;
    const char* static_ip;
    const char* netmask;
    const char* gateway;
} NetworkConfig;

/* Function prototypes */
extern int fprime_main(int argc, char* argv[]);
NetworkConfig* create_network_config(int use_dhcp, const char* static_ip, const char* netmask, const char* gateway);
void cleanup_network_config(NetworkConfig* config);
int initialize_network(NetworkConfig* config);

/* Helper to duplicate strings */
static char* my_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

/* Network configuration API functions */
NetworkConfig* create_network_config(
    int use_dhcp, 
    const char* static_ip, 
    const char* netmask, 
    const char* gateway
) {
    NetworkConfig* config = malloc(sizeof(NetworkConfig));
    if (config) {
        config->use_dhcp = use_dhcp;
        config->static_ip = static_ip ? my_strdup(static_ip) : NULL;
        config->netmask = netmask ? my_strdup(netmask) : NULL;
        config->gateway = gateway ? my_strdup(gateway) : NULL;
    }
    return config;
}

void cleanup_network_config(NetworkConfig* config) {
    if (config) {
        free((void*)config->static_ip);
        free((void*)config->netmask);
        free((void*)config->gateway);
        free(config);
    }
}

/* Initialize network for GR740 board */
int initialize_network(NetworkConfig* config) {
    printf("Network initialization is not yet implemented\n");
    return 0;
}

/* RTEMS Initial Task */
rtems_task Init(rtems_task_argument ignored) {
    printf("RTEMS initialized for F Prime on GR740\n");
    
    /* Call the F Prime main function */
    // char *argv[] = {"fprime-gr740", NULL};
    // int result = fprime_main(1, argv);
    
    // printf("F Prime application exited with code: %d\n", result);
    rtems_task_suspend(RTEMS_SELF);
}