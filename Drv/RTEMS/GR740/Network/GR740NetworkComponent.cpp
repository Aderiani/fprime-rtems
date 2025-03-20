// ======================================================================
// \title  GR740NetworkComponent.cpp
// \author [Your Name]
// \brief  cpp file for GR740NetworkComponent implementation class
//
// \copyright
// Copyright 2025, [Your Organization or Name].
// ALL RIGHTS RESERVED.
//
// ======================================================================
#define IFNAMSIZ IF_NAMESIZE // For compatibility with older versions of RTEMS

#include "Drv/RTEMS/GR740/Network/GR740NetworkComponent.hpp"
#include <Fw/Types/Assert.hpp>
#include <cstring> // For memset
#include <rtems.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/dhcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <grlib/greth.h>
#include <net/if.h> // For IF_NAMESIZE

namespace Drv {

static int greth_attach_wrapper(struct rtems_bsdnet_ifconfig* conf, int attaching) {
    greth_register_drv(); // Call the GRLIB function
    return 0; // Success
}



GR740NetworkComponent::GR740NetworkComponent(const char* const compName)
    : GR740NetworkComponentComponentBase(compName),
      m_networkActive(false),
      m_ipAddress("0.0.0.0"),
      m_netmask("0.0.0.0"),
      m_packetsSent(0),
      m_packetsReceived(0) {
    // Initialize GRETH hardware configuration
    m_grethConfig.base_address = 0x80000900; // Verify in GR740 docs
    m_grethConfig.vector = 13;
    m_grethConfig.txd_count = 16;
    m_grethConfig.rxd_count = 16;
}

void GR740NetworkComponent::init(const NATIVE_INT_TYPE instance) {
    GR740NetworkComponentComponentBase::init(instance);

    // Auto-start network if enabled
    bool autoStart = false;
    Fw::ParamValid valid;
    autoStart = paramGet_AUTO_START(valid);
    if (valid == Fw::ParamValid::VALID && autoStart) {
        Fw::String ipAddr, netmask, gateway;
        bool useDhcp = paramGet_DEFAULT_USE_DHCP(valid);
        ipAddr = paramGet_DEFAULT_IP_ADDRESS(valid);
        netmask = paramGet_DEFAULT_NETMASK(valid);
        gateway = paramGet_DEFAULT_GATEWAY(valid);
        if (valid == Fw::ParamValid::VALID) {
            startNetwork_handler(0, useDhcp, ipAddr, netmask, gateway);
        }
    }
}

// ----------------------------------------------------------------------
// Command handlers
// ----------------------------------------------------------------------

void GR740NetworkComponent::START_NETWORK_cmdHandler(
    const FwOpcodeType opCode, const U32 cmdSeq,
    bool useDhcp, Fw::StringBase& ipAddress, Fw::StringBase& netmask, Fw::StringBase& gateway) {
    // Setup network interface configuration
    memset(&m_ifconfig, 0, sizeof(struct rtems_bsdnet_ifconfig));
    m_ifconfig.name = const_cast<char*>("gr0");
    m_ifconfig.attach = greth_attach_wrapper; // Use the wrapper
    m_ifconfig.ip_address = useDhcp ? nullptr : const_cast<char*>(ipAddress.toChar());
    m_ifconfig.ip_netmask = useDhcp ? nullptr : const_cast<char*>(netmask.toChar());
    m_ifconfig.mtu = 0;
    m_ifconfig.rbuf_count = 16;
    m_ifconfig.xbuf_count = 8;
    m_ifconfig.drv_ctrl = static_cast<void*>(&m_grethConfig);

    // Setup network stack configuration
    memset(&m_bsdnetConfig, 0, sizeof(struct rtems_bsdnet_config));
    m_bsdnetConfig.ifconfig = &m_ifconfig;
    m_bsdnetConfig.bootp = useDhcp ? rtems_bsdnet_do_dhcp : nullptr;
    m_bsdnetConfig.network_task_priority = 100; // Correct field name
    m_bsdnetConfig.mbuf_bytecount = 65536;
    m_bsdnetConfig.mbuf_cluster_bytecount = 131072;
    m_bsdnetConfig.hostname = nullptr;
    m_bsdnetConfig.domainname = nullptr;
    m_bsdnetConfig.gateway = useDhcp ? nullptr : const_cast<char*>(gateway.toChar());
    m_bsdnetConfig.log_host = nullptr;
    m_bsdnetConfig.name_server[0] = 0;
    m_bsdnetConfig.ntp_server[0] = 0;
    m_bsdnetConfig.sb_efficiency = 2;
    m_bsdnetConfig.udp_tx_buf_size = 4096;
    m_bsdnetConfig.udp_rx_buf_size = 4096;
    m_bsdnetConfig.tcp_tx_buf_size = 8192;
    m_bsdnetConfig.tcp_rx_buf_size = 8192;

    // Initialize the network stack
    this->log_ACTIVITY_HI_NetworkInitSuccessful(useDhcp, ipAddress, netmask); // Log intent
    int status = rtems_bsdnet_initialize_network();
    if (status != 0) {
        this->log_WARNING_HI_NetworkInitFailed(status);
        m_networkActive = false;
        this->tlmWrite_NetworkStatus(false);
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    // Update state based on configuration
    if (useDhcp) {
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&m_ifconfig.ip_address);
        struct sockaddr_in* mask = reinterpret_cast<struct sockaddr_in*>(&m_ifconfig.ip_netmask);
        char ipStr[INET_ADDRSTRLEN];
        char maskStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &mask->sin_addr, maskStr, INET_ADDRSTRLEN);
        m_ipAddress = ipStr;
        m_netmask = maskStr;
    } else {
        m_ipAddress = ipAddress;
        m_netmask = netmask;
    }

    m_networkActive = true;
    this->log_ACTIVITY_HI_NetworkInitSuccessful(useDhcp, m_ipAddress, m_netmask);
    this->tlmWrite_NetworkStatus(true);
    this->tlmWrite_NetworkIpAddress(m_ipAddress);
    this->tlmWrite_NetworkNetmask(m_netmask);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void GR740NetworkComponent::RESET_NETWORK_cmdHandler(
    const FwOpcodeType opCode, const U32 cmdSeq) {
    this->log_ACTIVITY_HI_NetworkReset();

    // RTEMS BSD reinitialization as reset
    int status = rtems_bsdnet_initialize_network();
    bool success = (status == 0);

    m_networkActive = success;
    if (!success) {
        m_ipAddress = "0.0.0.0";
        m_netmask = "0.0.0.0";
    }

    this->log_ACTIVITY_HI_NetworkResetResult(success);
    this->tlmWrite_NetworkStatus(m_networkActive);
    this->tlmWrite_NetworkIpAddress(m_ipAddress);
    this->tlmWrite_NetworkNetmask(m_netmask);

    this->cmdResponse_out(opCode, cmdSeq, success ? Fw::CmdResponse::OK : Fw::CmdResponse::EXECUTION_ERROR);
}

// ----------------------------------------------------------------------
// Port handlers
// ----------------------------------------------------------------------

void GR740NetworkComponent::startNetwork_handler(
    const NATIVE_INT_TYPE portNum,
    bool useDhcp, Fw::StringBase& ipAddress,
    Fw::StringBase& netmask, Fw::StringBase& gateway) {
    START_NETWORK_cmdHandler(0, 0, useDhcp, ipAddress, netmask, gateway);
}

void GR740NetworkComponent::networkStatusGet_handler(
    const NATIVE_INT_TYPE portNum,
    bool& connected, Fw::StringBase& ipAddress, Fw::StringBase& netmask) {
    connected = m_networkActive;
    ipAddress = m_ipAddress;
    netmask = m_netmask;
}

// ----------------------------------------------------------------------
// Parameter handlers
// ----------------------------------------------------------------------

Fw::ParamValid GR740NetworkComponent::paramGet_handler(
    const NATIVE_INT_TYPE portNum, U32 id, Fw::ParamBuffer& val) {
    Fw::ParamValid valid = Fw::ParamValid::VALID;
    Fw::SerializeBufferBase& serialVal = val;

    switch (id) {
        case 0: // AUTO_START
            valid = (serialVal.serialize(paramGet_AUTO_START(valid)) == Fw::FW_SERIALIZE_OK) ? Fw::ParamValid::VALID : Fw::ParamValid::INVALID;
            break;
        case 1: // DEFAULT_USE_DHCP
            valid = (serialVal.serialize(paramGet_DEFAULT_USE_DHCP(valid)) == Fw::FW_SERIALIZE_OK) ? Fw::ParamValid::VALID : Fw::ParamValid::INVALID;
            break;
        case 2: // DEFAULT_IP_ADDRESS
            valid = (serialVal.serialize(paramGet_DEFAULT_IP_ADDRESS(valid)) == Fw::FW_SERIALIZE_OK) ? Fw::ParamValid::VALID : Fw::ParamValid::INVALID;
            break;
        case 3: // DEFAULT_NETMASK
            valid = (serialVal.serialize(paramGet_DEFAULT_NETMASK(valid)) == Fw::FW_SERIALIZE_OK) ? Fw::ParamValid::VALID : Fw::ParamValid::INVALID;
            break;
        case 4: // DEFAULT_GATEWAY
            valid = (serialVal.serialize(paramGet_DEFAULT_GATEWAY(valid)) == Fw::FW_SERIALIZE_OK) ? Fw::ParamValid::VALID : Fw::ParamValid::INVALID;
            break;
        default:
            valid = Fw::ParamValid::DEFAULT; // Use DEFAULT for unknown IDs
            break;
    }
    return valid;
}

GR740NetworkComponent::~GR740NetworkComponent() {
    // No cleanup needed; RTEMS BSD manages its own resources
}

} // namespace Drv