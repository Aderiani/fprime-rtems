// File: Drv/RTEMS/GR740/Network/NetworkPorts.fpp

module Drv {
  @ Port for starting the network
  port StartNetwork(
    useDhcp: bool @< Use DHCP flag
    ipAddress: string @< Static IP address if not using DHCP
    netmask: string @< Netmask if not using DHCP
    gateway: string @< Gateway address if not using DHCP
  )
  
  @ Port for retrieving network status
  port NetworkStatusGet(
    ref connected: bool @< Whether the network is connected
    ref ipAddress: string @< Current IP address
    ref netmask: string @< Current netmask
  )
}