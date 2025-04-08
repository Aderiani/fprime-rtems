// RTEMSInit/network_init.h
#ifndef _FPRIME_NETWORKCONFIG_H_
#define _FPRIME_NETWORKCONFIG_H_

struct FPrimeNetworkConfig {
    int use_dhcp;
    const char* static_ip;
    const char* netmask;
    const char* gateway;
    const char* hostname;
};

// Declare a default configuration
extern const struct FPrimeNetworkConfig default_network_config;

#ifdef __cplusplus
extern "C" {
#endif

// Network initialization function
int initialize_fprime_network();

#ifdef __cplusplus
}
#endif

#endif