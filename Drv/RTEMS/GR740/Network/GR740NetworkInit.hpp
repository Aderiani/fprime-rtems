#ifndef GR740_NETWORK_INIT_HPP
#define GR740_NETWORK_INIT_HPP

/**
 * Initialize the GR740 network stack
 * 
 * @param useDhcp True to use DHCP, false to use static IP
 * @param ipAddress Static IP address (ignored if useDhcp is true)
 * @param netmask Static netmask (ignored if useDhcp is true)
 * @return True if initialization succeeds, false otherwise
 */
bool initializeGR740Network(bool useDhcp = true, 
                            const char* ipAddress = "192.168.1.10", 
                            const char* netmask = "255.255.255.0");

#endif // GR740_NETWORK_INIT_HPP