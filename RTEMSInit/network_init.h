#ifndef NETWORK_INIT_H
#define NETWORK_INIT_H



typedef struct {
    char* hostname;
    char* domainname;
    char* gateway;
    char* ip_address;
    char* ip_netmask;
    // etc.
} NetworkConfig;

int initialize_network(NetworkConfig* config);
NetworkConfig* create_network_config(const char* hostname, const char* domainname, const char* gateway, const char* ip_address, const char* ip_netmask);
void cleanup_network_config(NetworkConfig* config);

#endif  // NETWORK_INIT_H