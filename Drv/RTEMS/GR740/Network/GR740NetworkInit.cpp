// ======================================================================
// \title  GR740NetworkInit.cpp
// \author fprime-community
// \brief  Networking initialization for GR740 RTEMS
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include "GR740NetworkInit.hpp"
#include <Fw/Logger/Logger.hpp>

#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/dhcp.h> // Include if using DHCP

// Include GR740 specific network headers
#include <bsp/greth.h> // for GRETH Ethernet driver

// Network configuration - must be global
rtems_bsdnet_config rtems_bsdnet_configuration;
rtems_bsdnet_ifconfig gr740_network_config;

// GRETH Hardware configuration
static const gr740_greth_configuration = {
    .base_address = 0x80000900, // Check your hardware documentation for correct address
    .vector = 13,               // Check your hardware documentation for correct IRQ vector
    .txd_count = 16,            // Number of transmit descriptors
    .rxd_count = 16             // Number of receive descriptors
};

// Networking initialization
bool initializeGR740Network(bool useDhcp, const char* ipAddress, const char* netmask) {
    // Configure network interface
    gr740_network_config = {
        .name = "gr0",              // Interface name
        .attach = gr740_greth_attach,  // Driver attach function
        .hwaddr = nullptr,          // MAC address will use hardware default
        .ipaddr = (char*)ipAddress, // IP address (if not using DHCP)
        .netmask = (char*)netmask,  // Netmask (if not using DHCP)
        .broadcast = nullptr,       // Broadcast address will be calculated
        .gateway = nullptr,         // No gateway set by default
        .mtu = 0,                   // Use driver default
        .rbuf_count = 8,            // Receive buffers
        .xbuf_count = 4,            // Transmit buffers
        .port = (void*)&gr740_greth_configuration, // Hardware configuration
        .next = nullptr,            // Only one interface
        .ctrl = nullptr,            // No special control
    };

    // Configure network stack
    rtems_bsdnet_configuration = {
        .ifconfig = &gr740_network_config, // Interface configuration
        .bootp = useDhcp ? rtems_bsdnet_do_dhcp : nullptr, // Use DHCP if requested
        .network_task_priority = 100,      // Network task priority
        .mbuf_bytecount = 65536,          // Memory for networking
        .mbuf_cluster_bytecount = 131072, // Memory for networking
        .hostname = nullptr,              // Hostname will be set from environment
        .domainname = nullptr,            // Domain name will be set from environment
        .gateway = nullptr,               // Gateway address
        .log_host = nullptr,              // No log host
        .name_server = {{0}},             // No DNS servers
        .ntp_server = {{0}},              // No NTP servers
        .sb_efficiency = 0,               // Use defaults
        .udp_tx_buf_size = 0,             // Use defaults
        .udp_rx_buf_size = 0,             // Use defaults
        .tcp_tx_buf_size = 0,             // Use defaults
        .tcp_rx_buf_size = 0,             // Use defaults
    };

    // Initialize network stack
    int status = rtems_bsdnet_initialize_network();
    if (status != 0) {
        Fw::Logger::logMsg("Failed to initialize network: error code %d\n", status);
        return false;
    }

    if (useDhcp) {
        // DHCP will have configured the interface
        char* actualIp = inet_ntoa(gr740_network_config.ip_address);
        char* actualNetmask = inet_ntoa(gr740_network_config.ip_netmask);
        Fw::Logger::logMsg("Network initialized with DHCP: IP %s, Netmask %s\n", 
                          actualIp, actualNetmask);
    } else {
        Fw::Logger::logMsg("Network initialized: IP %s, Netmask %s\n", 
                          ipAddress, netmask);
    }

    return true;
}