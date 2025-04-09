// ======================================================================
// \title  LedBlinkerTopology.cpp
// \brief  cpp file for LedBlinker topology
//
// \copyright
// Copyright 2009-2025, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// ======================================================================
#ifdef __rtems__
#include <rtems.h>  // For rtems_task_wake_after and other RTEMS functions
#endif

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

void cycleComponentsFunc(void*) {
    while (!cycleHalt) {
        // Call the ISR function to simulate a cycle
        blockDrv.callIsr();
        
        #ifdef __rtems__
        // Use RTEMS native delay for better timing
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 
                             cycleInterval.getSeconds() +
                             rtems_clock_get_ticks_per_second() * 
                             cycleInterval.getUSeconds() / 1000000);
        #else
        // Delay to the next cycle using F' Time
        Os::Task::delay(cycleInterval);
        #endif
    }
}


void configureHardwareTopology() {
    Fw::Logger::log("Hardware topology configured\n");
}


void setupTopology(const TopologyState& state) {
    // Initialize components one by one
    initComponents(state);
    setBaseIds();
    connectComponents();
    
    // Configure each component individually with error checking
    if (state.hostname != nullptr) {
        tcpServer.configure(state.hostname, state.port, 0, 100, 4096);
    } else {
        tcpServer.configure("0.0.0.0", 50000, 0, 100, 4096);
    }
    
    // Load parameters
    loadParameters();
    
    // Start tasks sequentially with delays between each
    startTasks(state);
    
    // Delay before starting TCP server
    Os::Task::delay(Fw::TimeInterval(0, 100000));  // 100ms delay
    
    // Start TCP server task
    Os::TaskString name("TcpServerRecv");
    tcpServer.start(name, 100, 64 * 1024);
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
    
    #ifdef __rtems__
    // For RTEMS, use a more basic approach to cycle timing
    Os::TaskString name("SimCycle");
    Os::Task::Arguments arguments(name, cycleComponentsFunc, nullptr, 
                               120, // Higher priority
                               64 * 1024); // Stack size
    #else
    // Original task setup for non-RTEMS
    Os::TaskString name("SimCycle");
    Os::Task::Arguments arguments(name, cycleComponentsFunc, nullptr);
    #endif
    
    simulatedCycleTask.start(arguments);
}

void stopSimulatedCycle() {
    cycleHalt = true;
    // Use the argument-free join method
    simulatedCycleTask.join();
}

} // namespace LedBlinker