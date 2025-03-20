// ======================================================================
// \title  GR740NetworkComponent.hpp
// \author [Your Name]
// \brief  hpp file for GR740NetworkComponent implementation class
//
// \copyright
// Copyright 2025, [Your Organization or Name].
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_RTEMS_GR740_NETWORK_COMPONENT_HPP
#define DRV_RTEMS_GR740_NETWORK_COMPONENT_HPP

#include "Drv/RTEMS/GR740/Network/GR740NetworkComponentComponentAc.hpp"
#include <rtems/rtems_bsdnet.h>
#include <grlib/greth.h>

// Define greth_hw_config based on GRLIB GRETH documentation
struct greth_hw_config {
    U32 base_address; // Base address of GRETH registers
    U32 vector;       // Interrupt vector
    U32 txd_count;    // Number of transmit descriptors
    U32 rxd_count;    // Number of receive descriptors
};

namespace Drv {

class GR740NetworkComponent : public GR740NetworkComponentComponentBase {
  public:
    GR740NetworkComponent(const char* const compName);
    void init(const NATIVE_INT_TYPE instance);
    ~GR740NetworkComponent();

  PRIVATE:
    // Command handlers
    void START_NETWORK_cmdHandler(
        const FwOpcodeType opCode, const U32 cmdSeq,
        bool useDhcp, Fw::StringBase& ipAddress, Fw::StringBase& netmask, Fw::StringBase& gateway);
    void RESET_NETWORK_cmdHandler(const FwOpcodeType opCode, const U32 cmdSeq);

    // Port handlers
    void startNetwork_handler(
        const NATIVE_INT_TYPE portNum,
        bool useDhcp, Fw::StringBase& ipAddress,
        Fw::StringBase& netmask, Fw::StringBase& gateway);
    void networkStatusGet_handler(
        const NATIVE_INT_TYPE portNum,
        bool& connected, Fw::StringBase& ipAddress, Fw::StringBase& netmask);

    // Parameter handler
    Fw::ParamValid paramGet_handler(
        const NATIVE_INT_TYPE portNum, U32 id, Fw::ParamBuffer& val);

    // Member variables
    bool m_networkActive;
    Fw::String m_ipAddress;
    Fw::String m_netmask;
    U32 m_packetsSent;
    U32 m_packetsReceived;

    // Network configuration
    struct rtems_bsdnet_config m_bsdnetConfig;
    struct rtems_bsdnet_ifconfig m_ifconfig;
    struct greth_hw_config m_grethConfig;
};

} // namespace Drv

#endif // DRV_RTEMS_GR740_NETWORK_COMPONENT_HPP