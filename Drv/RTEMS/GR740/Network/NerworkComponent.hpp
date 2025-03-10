// File: Drv/RTEMS/GR740/Network/NetworkComponent.hpp

#ifndef DRV_RTEMS_GR740_NETWORK_COMPONENT_HPP
#define DRV_RTEMS_GR740_NETWORK_COMPONENT_HPP

#include <Drv/RTEMS/GR740/Network/NetworkComponentAc.hpp>

namespace Drv {

  class GR740NetworkComponent : public GR740NetworkComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct GR740NetworkComponent
    //!
    GR740NetworkComponent(
        const char *const compName //!< Component name
    );

    //! Initialize GR740NetworkComponent
    //!
    void init(
        const FwIndexType instance = 0 //!< The instance number
    );

    //! Destroy GR740NetworkComponent
    //!
    ~GR740NetworkComponent();

  PRIVATE:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler for input port startNetwork
    //!
    void startNetwork_handler(
        const FwIndexType portNum, //!< The port number
        bool useDhcp,
        const Fw::StringBase& ipAddress,
        const Fw::StringBase& netmask,
        const Fw::StringBase& gateway
    ) override;

    //! Handler for input port networkStatusGet
    //!
    void networkStatusGet_handler(
        const FwIndexType portNum, //!< The port number
        bool &connected,
        Fw::StringBase& ipAddress,
        Fw::StringBase& netmask
    ) override;

    // ----------------------------------------------------------------------
    // Command handler implementations
    // ----------------------------------------------------------------------

    //! Implementation for START_NETWORK command handler
    //!
    void START_NETWORK_cmdHandler(
        const FwOpcodeType opCode, //!< The opcode
        const U32 cmdSeq, //!< The command sequence number
        bool useDhcp,
        const Fw::CmdStringArg& ipAddress,
        const Fw::CmdStringArg& netmask,
        const Fw::CmdStringArg& gateway
    ) override;

    //! Implementation for RESET_NETWORK command handler
    //!
    void RESET_NETWORK_cmdHandler(
        const FwOpcodeType opCode, //!< The opcode
        const U32 cmdSeq //!< The command sequence number
    ) override;

    // ----------------------------------------------------------------------
    // Parameter handlers
    // ----------------------------------------------------------------------
    
    //! Called when a parameter is updated
    //!
    void parameterUpdated(FwPrmIdType id) override;

    // ----------------------------------------------------------------------
    // Private methods
    // ----------------------------------------------------------------------
    
    //! Initialize the network
    //!
    bool doNetworkInit(
        bool useDhcp,
        const Fw::StringBase& ipAddress,
        const Fw::StringBase& netmask,
        const Fw::StringBase& gateway
    );

    // ----------------------------------------------------------------------
    // Private member variables
    // ----------------------------------------------------------------------
    
    bool m_networkInitialized; //!< Flag indicating if network has been initialized
    bool m_connected; //!< Flag indicating if network is connected
    bool m_useDhcp; //!< DHCP setting used
    char m_ipAddress[24]; //!< Current IP address
    char m_netmask[24]; //!< Current netmask
    char m_gateway[24]; //!< Current gateway
    U32 m_packetsSent; //!< Packet counter for telemetry
    U32 m_packetsReceived; //!< Packet counter for telemetry
  };

} // end namespace Drv

#endif // DRV_RTEMS_GR740_NETWORK_COMPONENT_HPP