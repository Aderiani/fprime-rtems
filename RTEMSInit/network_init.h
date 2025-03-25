/**
 * @file network_init.h
 * @brief Network initialization API for F Prime on RTEMS
 */
#ifndef NETWORK_INIT_H
#define NETWORK_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Network configuration structure
 */
typedef struct {
    int use_dhcp;
    const char* static_ip;
    const char* netmask;
    const char* gateway;
} NetworkConfig;

/**
 * @brief Initialize network with given configuration
 * 
 * @param config Network configuration
 * @return int 0 on success, non-zero on failure
 */
int initialize_network(NetworkConfig* config);

/**
 * @brief Create a network configuration object
 * 
 * @param use_dhcp Whether to use DHCP
 * @param static_ip Static IP address (if not using DHCP)
 * @param netmask Network mask (if not using DHCP)
 * @param gateway Gateway address (if not using DHCP)
 * @return NetworkConfig* Configuration object
 */
NetworkConfig* create_network_config(
    int use_dhcp, 
    const char* static_ip, 
    const char* netmask, 
    const char* gateway
);

/**
 * @brief Clean up network configuration
 * 
 * @param config Configuration to clean up
 */
void cleanup_network_config(NetworkConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_INIT_H */