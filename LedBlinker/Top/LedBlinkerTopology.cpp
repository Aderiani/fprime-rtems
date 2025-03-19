// ======================================================================
// \title  LedBlinkerTopology.cpp
// \brief  cpp file for LedBlinker topology
//
// \copyright
// Copyright 2009-2025, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// ======================================================================

#include "LedBlinkerTopology.hpp"
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Os/Task.hpp>

// Include generated topology headers (assumed from topology.fpp)
#include "LedBlinker/Top/LedBlinkerTopologyAc.hpp"

// Define instances from topology.fpp (adjust names as per your file)
namespace LedBlinker {
    extern Drv::Network network;    // GR740 network component
    extern Drv::TcpServer tcpServer;             // TCP server component
    extern Drv::TcpClient tcpClient;             // TCP client component
    extern Drv::Udp udp;                         // UDP component
}

namespace LedBlinker {

void initComponents(const TopologyState& /*state*/) {
    // Initialize components with their base IDs
    network.init(0x0100);
    tcpServer.init(0x0200);
    tcpClient.init(0x0300);
    udp.init(0x0400);
}

void configComponents(const TopologyState& /*state*/) {
    // Configure components (no TopologyState members assumed)
    network.START_NETWORK(true, "0.0.0.0", "0.0.0.0", "0.0.0.0"); // DHCP
    tcpServer.configure(nullptr, 5000, 2, 0, 4096); // Bind to all interfaces, port 5000
    tcpClient.configure("192.168.1.100", 5000, 2, 0, 4096); // Connect to server (adjust IP)
    udp.configureRecv(nullptr, 5001, 4096); // Receive on port 5001
}

void configureHardwareTopology() {
    // No TaskConfig needed; network is started in configComponents
    // Remove placeholder GPIO logic for now (add back if you have a GPIO driver)
    Fw::Logger::log("Hardware topology configured\n");
}

void regCommands() {
    // Register commands for components that support it
    network.regCommands();
    tcpServer.regCommands();
    tcpClient.regCommands();
    udp.regCommands();
}

void setupTopology(const TopologyState& state) {
    initComponents(state);
    configComponents(state);
    configureHardwareTopology();
    regCommands();

    // Start tasks with Os::TaskString
    Os::TaskString name("TcpServerRecv");
    tcpServer.start(name, 100, Default::STACK_SIZE);

    name = Os::TaskString("TcpClientRecv");
    tcpClient.start(name, 100, Default::STACK_SIZE);

    name = Os::TaskString("UdpRecv");
    udp.start(name, 100, Default::STACK_SIZE);
}

void teardownTopology(const TopologyState& /*state*/) {
    // Stop tasks
    tcpServer.stop();
    tcpClient.stop();
    udp.stop();

    // Exit components
    network.exit();
    tcpServer.exit();
    tcpClient.exit();
    udp.exit();
}

} // namespace LedBlinker