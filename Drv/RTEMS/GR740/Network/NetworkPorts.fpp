
module Drv {
  @ Port for starting the network
  port GR740InitNetwork(
    useDhcp: bool @< Use DHCP flag
    ipAddress: string @< Static IP address if not using DHCP
    netmask: string @< Netmask if not using DHCP
    gateway: string @< Gateway address if not using DHCP
  )
  
  @ Port for retrieving network status
  port GR740NetworkStatus(
    ref connected: bool @< Whether the network is connected
    ref ipAddress: string @< Current IP address
    ref netmask: string @< Current netmask
  )
}