// File: Drv/RTEMS/GR740/Network/NetworkComponent.fpp

module Drv {
  @ A component that initializes and manages the GR740 network interface
  active component GR740NetworkComponent {
    
    # ----------------------------------------------------------------------
    # General ports
    # ----------------------------------------------------------------------
    
    @ Command receive port
    command recv port cmdIn
    
    @ Command registration port
    command reg port cmdRegOut
    
    @ Command response port
    command resp port cmdResponseOut

    @ Port for starting the network with given configurations
    async input port startNetwork: Drv.StartNetwork
    
    @ Port for checking network status
    sync input port networkStatusGet: Drv.NetworkStatusGet
    
    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------
    
    @ Telemetry port
    telemetry port tlmOut
    
    @ Event port
    event port eventOut
    
    @ Text event port
    text event port textEventOut
    
    @ Time get port
    time get port timeGetOut
    
    # ----------------------------------------------------------------------
    # Commands
    # ----------------------------------------------------------------------
    
    @ Command to start the network
    async command START_NETWORK(
      useDhcp: bool @< Use DHCP flag
      ipAddress: string size 20 @< Static IP address if not using DHCP
      netmask: string size 20 @< Netmask if not using DHCP
      gateway: string size 20 @< Gateway address if not using DHCP
    )
    
    @ Command to reset the network
    async command RESET_NETWORK
    
    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    
    @ Network initialization successful event
    event NetworkInitSuccessful(
      useDhcp: bool @< Whether DHCP was used
      ipAddress: string size 20 @< IP address assigned or configured
      netmask: string size 20 @< Netmask assigned or configured
    ) severity activity high format "Network initialized successfully: DHCP={} IP={} Netmask={}"
    
    @ Network initialization failed event
    event NetworkInitFailed(
      errorCode: I32 @< Error code from initialization
    ) severity warning high format "Network initialization failed with error code {}"
    
    @ Network reset event
    event NetworkReset() severity activity high format "Network reset initiated"
    
    @ Network reset result event
    event NetworkResetResult(
      success: bool @< Whether reset was successful
    ) severity activity high format "Network reset completed with status: {}"
    
    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------
    
    @ Network status
    telemetry NetworkStatus: bool id 0 format "Connected={}"
    
    @ Network IP address
    telemetry NetworkIpAddress: string size 20 id 1 update on change format "{}"
    
    @ Network netmask
    telemetry NetworkNetmask: string size 20 id 2 update on change format "{}"
    
    @ Packets sent
    telemetry PacketsSent: U32 id 3 format "{}"
    
    @ Packets received
    telemetry PacketsReceived: U32 id 4 format "{}"
    
    # ----------------------------------------------------------------------
    # Parameters
    # ----------------------------------------------------------------------
    
    @ Auto-start network on initialization
    param AUTO_START: bool id 0 default true
    
    @ Default DHCP setting
    param DEFAULT_USE_DHCP: bool id 1 default true
    
    @ Default IP address
    param DEFAULT_IP_ADDRESS: string size 20 id 2 default "192.168.1.10"
    
    @ Default netmask
    param DEFAULT_NETMASK: string size 20 id 3 default "255.255.255.0"
    
    @ Default gateway
    param DEFAULT_GATEWAY: string size 20 id 4 default ""
  }
}