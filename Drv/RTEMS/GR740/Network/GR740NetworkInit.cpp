// ======================================================================
// \title  GR740NetworkInit.cpp
// \author fprime-community
// \brief  Networking initialization for GR740 RTEMS
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include "GR740NetworkInit.hpp"
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/dhcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Include GR740 specific network headers
#include <bsp/greth.h>

// Global network configuration (must be global for RTEMS)
rtems_bsdnet_config rtems_bsdnet_configuration;
rtems_bsdnet_ifconfig rtems_bsdnet_ifconfig;

// GRETH hardware configuration
struct greth_hw_config gr740_greth_config = {
    .base_address = 0x80000900,  // Check hardware documentation for correct address
    .vector = 13,                // Interrupt vector number
    .txd_count = 16,             // Transmit descriptors
    .rxd_count = 16              // Receive descriptors
};

namespace Drv {
namespace RTEMS {
namespace Network {

bool initializeGR740Network(bool useDhcp, const char* ipAddress, const char* netmask, const char* gateway) {
    Fw::Logger::logMsg("Initializing GR740 network...\n");
    
    // Setup network interface configuration
    memset(&rtems_bsdnet_ifconfig, 0, sizeof(rtems_bsdnet_ifconfig));
    rtems_bsdnet_ifconfig.name = const_cast<char*>("gr0");
    rtems_bsdnet_ifconfig.attach = greth_attach;
    rtems_bsdnet_ifconfig.hwaddr = nullptr; // Use hardware default MAC
    rtems_bsdnet_ifconfig.ipaddr = useDhcp ? nullptr : const_cast<char*>(ipAddress);
    rtems_bsdnet_ifconfig.netmask = useDhcp ? nullptr : const_cast<char*>(netmask);
    rtems_bsdnet_ifconfig.broadcast = nullptr;
    rtems_bsdnet_ifconfig.gateway = nullptr;
    rtems_bsdnet_ifconfig.mtu = 0;
    rtems_bsdnet_ifconfig.rbuf_count = 16;
    rtems_bsdnet_ifconfig.xbuf_count = 8;
    rtems_bsdnet_ifconfig.port = static_cast<void*>(&gr740_greth_config);
    rtems_bsdnet_ifconfig.next = nullptr;
    rtems_bsdnet_ifconfig.ctrl = nullptr;
    
    // Setup network stack configuration
    memset(&rtems_bsdnet_configuration, 0, sizeof(rtems_bsdnet_configuration));
    rtems_bsdnet_configuration.ifconfig = &rtems_bsdnet_ifconfig;
    rtems_bsdnet_configuration.bootp = useDhcp ? rtems_bsdnet_do_dhcp : nullptr;
    rtems_bsdnet_configuration.network_task_priority = 100;
    rtems_bsdnet_configuration.mbuf_bytecount = 65536;
    rtems_bsdnet_configuration.mbuf_cluster_bytecount = 131072;
    rtems_bsdnet_configuration.hostname = nullptr;
    rtems_bsdnet_configuration.domainname = nullptr;
    rtems_bsdnet_configuration.gateway = useDhcp ? nullptr : const_cast<char*>(gateway);
    rtems_bsdnet_configuration.log_host = nullptr;
    rtems_bsdnet_configuration.name_server[0] = 0;
    rtems_bsdnet_configuration.ntp_server[0] = 0;
    rtems_bsdnet_configuration.sb_efficiency = 2;
    rtems_bsdnet_configuration.udp_tx_buf_size = 4096;
    rtems_bsdnet_configuration.udp_rx_buf_size = 4096;
    rtems_bsdnet_configuration.tcp_tx_buf_size = 8192;
    rtems_bsdnet_configuration.tcp_rx_buf_size = 8192;
    
    // Initialize the network stack
    Fw::Logger::logMsg("Starting network initialization...\n");
    int status = rtems_bsdnet_initialize_network();
    if (status != 0) {
        Fw::Logger::logMsg("Network initialization failed with error: %d\n", status);
        return false;
    }
    
    // Log network configuration
    if (useDhcp) {
        // Get the DHCP assigned address
        struct sockaddr_in* addr = (struct sockaddr_in*)&rtems_bsdnet_ifconfig.ip_address;
        struct sockaddr_in* mask = (struct sockaddr_in*)&rtems_bsdnet_ifconfig.ip_netmask;
        
        char ipStr[INET_ADDRSTRLEN];
        char maskStr[INET_ADDRSTRLEN];
        
        inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &mask->sin_addr, maskStr, INET_ADDRSTRLEN);
        
        Fw::Logger::logMsg("Network initialized with DHCP: IP %s, Netmask %s\n", ipStr, maskStr);
    } else {
        Fw::Logger::logMsg("Network initialized with static config: IP %s, Netmask %s\n", ipAddress, netmask);
        if (gateway && gateway[0] != '\0') {
            Fw::Logger::logMsg("Gateway: %s\n", gateway);
        }
    }
    
    return true;
}

}  // namespace Network
}  // namespace RTEMS
}  // namespace Drv