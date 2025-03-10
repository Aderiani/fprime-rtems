// ======================================================================
// \title  GR740NetworkInit.hpp
// \author fprime-community
// \brief  Networking initialization for GR740 RTEMS
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_RTEMS_GR740_NETWORK_INIT_HPP
#define DRV_RTEMS_GR740_NETWORK_INIT_HPP

#include <FpConfig.hpp>

namespace Drv {
namespace RTEMS {
namespace Network {

/**
 * Initialize the GR740 network stack
 * 
 * This function initializes the RTEMS networking stack for the GR740
 * GRETH Ethernet controller. It configures the network interface
 * with either static IP settings or using DHCP.
 * 
 * @param useDhcp True to use DHCP, false to use static IP settings
 * @param ipAddress Static IP address (ignored if useDhcp is true)
 * @param netmask Static netmask (ignored if useDhcp is true)
 * @param gateway Static gateway (ignored if useDhcp is true)
 * @return True if initialization succeeds, false otherwise
 */
bool initializeGR740Network(
    bool useDhcp = true,
    const char* ipAddress = "192.168.1.10",
    const char* netmask = "255.255.255.0",
    const char* gateway = ""
);

}  // namespace Network
}  // namespace RTEMS
}  // namespace Drv

#endif // DRV_RTEMS_GR740_NETWORK_INIT_HPP