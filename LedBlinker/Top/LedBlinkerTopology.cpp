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

#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Mutex.hpp>
#include <Os/Task.hpp>
#include "LedBlinkerTopology.hpp"

// Include generated topology headers
#include "LedBlinker/Top/LedBlinkerTopologyAc.hpp"
#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/FramingProtocol/FprimeProtocol.hpp>

namespace LedBlinker {

// Global variables to track simulated cycle
static bool cycleHalt = false;
static Fw::TimeInterval cycleInterval(1, 0);  // 1 Hz default, changed by startSimulatedCycle
static Os::Task simulatedCycleTask;


// The reference topology uses a malloc-based allocator for components that need to allocate memory during the
// initialization phase.
Fw::MallocAllocator mallocator;

// The reference topology uses the F´ packet protocol when communicating with the ground and therefore uses the F´
// framing and deframing implementations.
Svc::FprimeFraming framing;
Svc::FprimeDeframing deframing;

Svc::ComQueue::QueueConfigurationTable configurationTable;

Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {2, 0}, {4, 0}}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
NATIVE_INT_TYPE rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
NATIVE_INT_TYPE rateGroup2Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
NATIVE_INT_TYPE rateGroup3Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

// A number of constants are needed for construction of the topology. These are specified here.
enum TopologyConstants {
    CMD_SEQ_BUFFER_SIZE = 4 * 1024,
    FILE_DOWNLINK_TIMEOUT = 1000,
    FILE_DOWNLINK_COOLDOWN = 1000,
    FILE_DOWNLINK_CYCLE_TIME = 1000,
    FILE_DOWNLINK_FILE_QUEUE_DEPTH = 10,
    HEALTH_WATCHDOG_CODE = 0x123,
    COMM_PRIORITY = 49,
    // bufferManager constants
    FRAMER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE + sizeof(U32)) + HASH_DIGEST_LENGTH +
                         Svc::FpFrameHeader::SIZE,
    FRAMER_BUFFER_COUNT = 30,
    DEFRAMER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE + sizeof(U32)),
    DEFRAMER_BUFFER_COUNT = 30,
    COM_DRIVER_BUFFER_SIZE = 5000,
    COM_DRIVER_BUFFER_COUNT = 30,
    BUFFER_MANAGER_ID = 200
};



void cycleComponentsFunc(void*) {
    while (!cycleHalt) {
        // Call the ISR function to simulate a cycle
        blockDrv.callIsr();

#ifdef __rtems__
        // Use RTEMS native delay for better timing
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() * cycleInterval.getSeconds() +
                              rtems_clock_get_ticks_per_second() * cycleInterval.getUSeconds() / 1000000);
#else
        // Delay to the next cycle using F' Time
        Os::Task::delay(cycleInterval);
#endif
    }
}


// Ping entries are autocoded, however; this code is not properly exported. Thus, it is copied here.
Svc::Health::PingEntry pingEntries[] = {
    {PingEntries::LedBlinker_blockDrv::WARN, PingEntries::LedBlinker_blockDrv::FATAL, "blockDrv"},
    {PingEntries::LedBlinker_tlmSend::WARN, PingEntries::LedBlinker_tlmSend::FATAL, "chanTlm"},
    {PingEntries::LedBlinker_cmdDisp::WARN, PingEntries::LedBlinker_cmdDisp::FATAL, "cmdDisp"},
    {PingEntries::LedBlinker_cmdSeq::WARN, PingEntries::LedBlinker_cmdSeq::FATAL, "cmdSeq"},
    {PingEntries::LedBlinker_eventLogger::WARN, PingEntries::LedBlinker_eventLogger::FATAL,
     "eventLogger"},
    {PingEntries::LedBlinker_fileDownlink::WARN, PingEntries::LedBlinker_fileDownlink::FATAL,
     "fileDownlink"},
    {PingEntries::LedBlinker_fileManager::WARN, PingEntries::LedBlinker_fileManager::FATAL,
     "fileManager"},
    {PingEntries::LedBlinker_fileUplink::WARN, PingEntries::LedBlinker_fileUplink::FATAL,
     "fileUplink"},
    {PingEntries::LedBlinker_prmDb::WARN, PingEntries::LedBlinker_prmDb::FATAL, "prmDb"},
    {PingEntries::LedBlinker_rateGroup1::WARN, PingEntries::LedBlinker_rateGroup1::FATAL,
     "rateGroup1"},
    {PingEntries::LedBlinker_rateGroup2::WARN, PingEntries::LedBlinker_rateGroup2::FATAL,
     "rateGroup2"},
    {PingEntries::LedBlinker_rateGroup3::WARN, PingEntries::LedBlinker_rateGroup3::FATAL,
     "rateGroup3"},
};

void configureTopology() {
   // Buffer managers need a configured set of buckets and an allocator used to allocate memory for those buckets.
   Svc::BufferManager::BufferBins upBuffMgrBins;
   memset(&upBuffMgrBins, 0, sizeof(upBuffMgrBins));
   upBuffMgrBins.bins[0].bufferSize = FRAMER_BUFFER_SIZE;
   upBuffMgrBins.bins[0].numBuffers = FRAMER_BUFFER_COUNT;
   upBuffMgrBins.bins[1].bufferSize = DEFRAMER_BUFFER_SIZE;
   upBuffMgrBins.bins[1].numBuffers = DEFRAMER_BUFFER_COUNT;
   upBuffMgrBins.bins[2].bufferSize = COM_DRIVER_BUFFER_SIZE;
   upBuffMgrBins.bins[2].numBuffers = COM_DRIVER_BUFFER_COUNT;
   bufferManager.setup(BUFFER_MANAGER_ID, 0, mallocator, upBuffMgrBins);

   // Framer and Deframer components need to be passed a protocol handler
   framer.setup(framing);
   deframer.setup(deframing);

   // Command sequencer needs to allocate memory to hold contents of command sequences
   cmdSeq.allocateBuffer(0, mallocator, CMD_SEQ_BUFFER_SIZE);

   // Rate group driver needs a divisor list
   rateGroupDriver.configure(rateGroupDivisorsSet);

   // Rate groups require context arrays.
   rateGroup1.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));
   rateGroup2.configure(rateGroup2Context, FW_NUM_ARRAY_ELEMENTS(rateGroup2Context));
   rateGroup3.configure(rateGroup3Context, FW_NUM_ARRAY_ELEMENTS(rateGroup3Context));

   // File downlink requires some project-derived properties.
   fileDownlink.configure(FILE_DOWNLINK_TIMEOUT, FILE_DOWNLINK_COOLDOWN, FILE_DOWNLINK_CYCLE_TIME,
                          FILE_DOWNLINK_FILE_QUEUE_DEPTH);

   // Parameter database is configured with a database file name, and that file must be initially read.
   prmDb.configure("PrmDb.dat");
   prmDb.readParamFile();

   // Health is supplied a set of ping entires.
   health.setPingEntries(pingEntries, FW_NUM_ARRAY_ELEMENTS(pingEntries), HEALTH_WATCHDOG_CODE);

   // Note: Uncomment when using Svc:TlmPacketizer
   // tlmSend.setPacketList(LedBlinkerPacketsPkts, LedBlinkerPacketsIgnore, 1);

   // Events (highest-priority)
   configurationTable.entries[0] = {.depth = 100, .priority = 0};
   // Telemetry
   configurationTable.entries[1] = {.depth = 500, .priority = 2};
   // File Downlink
   configurationTable.entries[2] = {.depth = 100, .priority = 1};
   // Allocation identifier is 0 as the MallocAllocator discards it
   comQueue.configure(configurationTable, 0, mallocator);


    tcpServer.configure("192.168.0.67", 50000, 0, 100, 4 * 1024);

}



void setupTopology(const TopologyState& state) {
    // Initialize components one by one
    initComponents(state);
    setBaseIds();
    connectComponents();
    configComponents(state);
    // Deployment-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);

    // Delay before starting TCP server
    Os::Task::delay(Fw::TimeInterval(0, 100000));  // 100ms delay

    // Start TCP server task
    Os::TaskString name("RTEMS_TcpServer");
    tcpServer.start(name, 100, 4 * 1024);
}

void teardownTopology(const TopologyState& state) {
    // Stop the TCP server task
    tcpServer.stop();
    tcpServer.join();  // Wait for task to exit

    // Stop tasks - this call is generated automatically
    stopTasks(state);

    // Free threads - this call is generated automatically
    freeThreads(state);
}


//TODO: Impelement hardware clock instead of simulated clock
// void startSimulatedCycle(Fw::TimeInterval interval) {
//     // Use a much slower cycle for RTEMS to reduce CPU load
//     // If interval is the default 1 second, use 2 seconds instead
//     if (interval.getSeconds() == 1 && interval.getUSeconds() == 0) {
//         interval = Fw::TimeInterval(2, 0);
//     }

//     cycleInterval = interval;
//     cycleHalt = false;

// // #ifdef __rtems__
// //     // For RTEMS, use a more basic approach to cycle timing
// //     Os::TaskString name("SimCycle");
// //     Os::Task::Arguments arguments(name, cycleComponentsFunc, nullptr,
// //                                   120,         // Higher priority
// //                                   64 * 1024);  // Stack size
// // #else
//     // Original task setup for non-RTEMS
//     Os::TaskString name("SimCycle");
//     Os::Task::Arguments arguments(name, cycleComponentsFunc, nullptr);
// // #endif

//     simulatedCycleTask.start(arguments);
// }

// void stopSimulatedCycle() {
//     cycleHalt = true;
//     // Use the argument-free join method
//     simulatedCycleTask.join();
// }
// Variables used for cycle simulation
Os::Mutex cycleLock;
volatile bool cycleFlag = true;

void startSimulatedCycle(Fw::TimeInterval interval) {
    cycleLock.lock();
    bool cycling = cycleFlag;
    cycleLock.unLock();

    // Main loop
    while (cycling) {
        LedBlinker::blockDrv.callIsr();
        Os::Task::delay(interval);

        cycleLock.lock();
        cycling = cycleFlag;
        cycleLock.unLock();
    }
}

void stopSimulatedCycle() {
    cycleLock.lock();
    cycleFlag = false;
    cycleLock.unLock();
}





}  // namespace LedBlinker