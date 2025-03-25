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

// Include generated topology headers
#include "LedBlinker/Top/LedBlinkerTopologyAc.hpp"

namespace LedBlinker {

// Global variables to track simulated cycle
static bool cycleHalt = false;
static Fw::TimeInterval cycleInterval(1,0); // 1 Hz default, changed by startSimulatedCycle
static Os::Task simulatedCycleTask;

// Thread for simulated cycling
void cycleComponentsFunc(void*) {
    while (!cycleHalt) {
        // Call the ISR function to simulate a cycle
        blockDrv.callIsr();
        
        // Delay to the next cycle
        Os::Task::delay(cycleInterval);
    }
}



void configureHardwareTopology() {
    Fw::Logger::log("Hardware topology configured\n");
}

void setupTopology(const TopologyState& state) {
    // Set up IDs - this call is generated automatically
    initComponents(state);
    setBaseIds();
    
    // Connect components - this call is generated automatically
    connectComponents();
    
    // Configure components

        // Configure the TCP server component
        if (state.hostname != nullptr) {
            tcpServer.configure(state.hostname, state.port, 0, 100, 4096);
        } else {
            tcpServer.configure("0.0.0.0", 50000, 0, 100, 4096); // Default values
        }
        
        // Uncomment and modify if you have GPIO driver config needed
        // if (gpioDriver.initialize()) {
        //     gpioDriver.configurePin(0, Drv::GR740GpioDriver::GPIO_DIRECTION_OUTPUT, Fw::Logic::LOW);
        // }
    
    // Configure hardware topology
    configureHardwareTopology();
    
    // Load parameters - this call is generated automatically
    loadParameters();
    
    // Start active component tasks - this call is generated automatically
    startTasks(state);
    
    // Start the TCP server task for receiving data
    Os::TaskString name("TcpServerRecv");
    tcpServer.start(name, 100, 64 * 1024); // Default stack size from instances.fpp
}

void teardownTopology(const TopologyState& state) {
    // Stop the TCP server task
    tcpServer.stop();
    tcpServer.join(); // Wait for task to exit
    
    // Stop tasks - this call is generated automatically
    stopTasks(state);
    
    // Free threads - this call is generated automatically
    freeThreads(state);
}

void startSimulatedCycle(Fw::TimeInterval interval) {
    cycleInterval = interval;
    cycleHalt = false;
    
    // Create and start the task with the newer Arguments-based approach
    Os::TaskString name("SimCycle");
    Os::Task::Arguments arguments(name, cycleComponentsFunc, nullptr);
    simulatedCycleTask.start(arguments);
}

void stopSimulatedCycle() {
    cycleHalt = true;
    // Use the argument-free join method
    simulatedCycleTask.join();
}

} // namespace LedBlinker