// File: Drv/RTEMS/GR740/Network/NetworkComponent.cpp

#include <Drv/RTEMS/GR740/Network/NetworkComponentAc.hpp>
#include <Drv/RTEMS/GR740/Network/GR740NetworkInit.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>

namespace Drv {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  GR740NetworkComponent ::
    GR740NetworkComponent(
        const char *const compName
    ) : GR740NetworkComponentBase(compName),
        m_networkInitialized(false),
        m_connected(false),
        m_packetsSent(0),
        m_packetsReceived(0)
  {
    // Initialize string members
    m_ipAddress[0] = '\0';
    m_netmask[0] = '\0';
  }

  void GR740NetworkComponent ::
    init(
        const FwIndexType instance
    )
  {
    GR740NetworkComponentBase::init(instance);
  }

  GR740NetworkComponent ::
    ~GR740NetworkComponent()
  {
    // No specific cleanup needed
  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void GR740NetworkComponent ::
    startNetwork_handler(
        const FwIndexType portNum,
        bool useDhcp,
        const Fw::StringBase& ipAddress,
        const Fw::StringBase& netmask,
        const Fw::StringBase& gateway
    )
  {
    // Forward to command implementation
    this->doNetworkInit(useDhcp, ipAddress, netmask, gateway);
  }

  void GR740NetworkComponent ::
    networkStatusGet_handler(
        const FwIndexType portNum,
        bool &connected,
        Fw::StringBase& ipAddress,
        Fw::StringBase& netmask
    )
  {
    // Return current status
    connected = m_connected;
    ipAddress = m_ipAddress;
    netmask = m_netmask;
  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void GR740NetworkComponent ::
    START_NETWORK_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        bool useDhcp,
        const Fw::CmdStringArg& ipAddress,
        const Fw::CmdStringArg& netmask,
        const Fw::CmdStringArg& gateway
    )
  {
    // Perform network initialization
    bool success = this->doNetworkInit(useDhcp, ipAddress, netmask, gateway);
    
    // Send command response
    this->cmdResponse_out(opCode, cmdSeq, success ? Fw::CmdResponse::OK : Fw::CmdResponse::EXECUTION_ERROR);
  }

  void GR740NetworkComponent ::
    RESET_NETWORK_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // Log the reset attempt
    this->log_ACTIVITY_HI_NetworkReset();
    
    // Reset network by re-initializing with the current parameters
    bool success = this->doNetworkInit(
        m_useDhcp,
        Fw::String(m_ipAddress),
        Fw::String(m_netmask),
        Fw::String(m_gateway)
    );
    
    // Log the result
    this->log_ACTIVITY_HI_NetworkResetResult(success);
    
    // Send command response
    this->cmdResponse_out(opCode, cmdSeq, success ? Fw::CmdResponse::OK : Fw::CmdResponse::EXECUTION_ERROR);
  }

  // ----------------------------------------------------------------------
  // Private helper methods
  // ----------------------------------------------------------------------

  bool GR740NetworkComponent ::
    doNetworkInit(
        bool useDhcp,
        const Fw::StringBase& ipAddress,
        const Fw::StringBase& netmask,
        const Fw::StringBase& gateway
    )
  {
    // Store the configuration
    m_useDhcp = useDhcp;
    strncpy(m_ipAddress, ipAddress.toChar(), sizeof(m_ipAddress)-1);
    m_ipAddress[sizeof(m_ipAddress)-1] = '\0';
    
    strncpy(m_netmask, netmask.toChar(), sizeof(m_netmask)-1);
    m_netmask[sizeof(m_netmask)-1] = '\0';
    
    strncpy(m_gateway, gateway.toChar(), sizeof(m_gateway)-1);
    m_gateway[sizeof(m_gateway)-1] = '\0';
    
    // Initialize the network
    bool success = Drv::RTEMS::Network::initializeGR740Network(
        useDhcp,
        m_ipAddress,
        m_netmask,
        m_gateway
    );
    
    // Update status
    m_networkInitialized = success;
    m_connected = success;
    
    // Update telemetry
    this->tlmWrite_NetworkStatus(m_connected);
    this->tlmWrite_NetworkIpAddress(Fw::String(m_ipAddress));
    this->tlmWrite_NetworkNetmask(Fw::String(m_netmask));
    
    // Log the result
    if (success) {
        this->log_ACTIVITY_HI_NetworkInitSuccessful(useDhcp, Fw::String(m_ipAddress), Fw::String(m_netmask));
    } else {
        this->log_WARNING_HI_NetworkInitFailed(-1);
    }
    
    return success;
  }

  // ----------------------------------------------------------------------
  // Parameter handlers
  // ----------------------------------------------------------------------

  void GR740NetworkComponent ::
    parameterUpdated(FwPrmIdType id)
  {
    // Check if we need to auto-start the network
    if (id == GR740NetworkComponentBase::PARAMID_AUTO_START) {
        Fw::ParamValid valid;
        bool autoStart = this->paramGet_AUTO_START(valid);
        
        if (valid == Fw::ParamValid::VALID && autoStart && !m_networkInitialized) {
            // Auto-start the network with default parameters
            Fw::ParamValid valid1, valid2, valid3, valid4;
            bool useDhcp = this->paramGet_DEFAULT_USE_DHCP(valid1);
            Fw::String ipAddress = this->paramGet_DEFAULT_IP_ADDRESS(valid2);
            Fw::String netmask = this->paramGet_DEFAULT_NETMASK(valid3);
            Fw::String gateway = this->paramGet_DEFAULT_GATEWAY(valid4);
            
            if (valid1 == Fw::ParamValid::VALID &&
                valid2 == Fw::ParamValid::VALID &&
                valid3 == Fw::ParamValid::VALID &&
                valid4 == Fw::ParamValid::VALID) {
                this->doNetworkInit(useDhcp, ipAddress, netmask, gateway);
            }
        }
    }
  }

} // end namespace Drv