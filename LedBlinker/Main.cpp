// ======================================================================
// \title  Main.cpp
// \brief Main program for F' application
// ======================================================================

#include <signal.h>
#include <Fw/Logger/Logger.hpp>
#include <LedBlinker/Top/LedBlinkerTopology.hpp>
#include <Os/Os.hpp>
#include <cstdlib>  // For atoi
#include <cstring>  // For strcmp

#include <rtems.h>
#include <rtems/rtems/clock.h>
#include <time.h>
// Add this for direct console output that doesn't rely on OS services
#include <stdio.h>

// External network initialization functions (optional, only for RTEMS)
#ifdef __rtems__
extern "C" {
void system_init();
}
#endif

extern "C" {
#include "RTEMSInit/network_init.h"
void initialize_rtems_clock(void);

// void checkResources() {
//     rtems_resource_snapshot snapshot;
//     rtems_resource_snapshot_take(&snapshot);

//     printf("RTEMS Resources: Tasks: %d/%d, Semaphores: %d/%d\n", snapshot.tasks_count, CONFIGURE_MAXIMUM_TASKS,
//            snapshot.semaphores_count, CONFIGURE_MAXIMUM_SEMAPHORES);
// }
}

// static void signalHandler(int signum) {
//     LedBlinker::stopSimulatedCycle();
// }

extern "C" int fprime_main(int argc, char* argv[]) {
    // Existing F' initialization code...
    Os::init();

    initialize_rtems_clock();

    // Initialize network
    if (initialize_fprime_network() != 0) {
        printf("Failed to initialize network\n");
        // return -1;
    }

    LedBlinker::TopologyState inputs;

    inputs.hostname = "192.168.0.67";
    inputs.port = 50000;  // Default port

    // Setup topology

    LedBlinker::setupTopology(inputs);

    // // Setup program shutdown via Ctrl-C
    // signal(SIGINT, signalHandler);
    // signal(SIGTERM, signalHandler);
    // (void)printf("Hit Ctrl-C to quit\n");

    // In LedBlinker/Main.cpp main loop:
    volatile bool keep_running = true;
    unsigned int counter = 0;

    // printf("\n=== Time Configuration Debug ===\n");
    // printf("Time serialized size: %d bytes\n", Fw::Time::SERIALIZED_SIZE);
    // printf("FwPacketDescriptorType: %zu bytes\n", sizeof(FwPacketDescriptorType));
    // printf("FwEventIdType: %zu bytes\n", sizeof(FwEventIdType));
    // printf("LogSeverity: %zu bytes\n", sizeof(Fw::LogSeverity));

    // // Test time creation
    // Fw::Time testTime(TimeBase::TB_WORKSTATION_TIME, 0, 1750000000, 123456);
    // printf("\nTest time: base=%d, ctx=%d, sec=%u, usec=%u\n", static_cast<int>(testTime.getTimeBase()),
    //        static_cast<int>(testTime.getContext()), testTime.getSeconds(), testTime.getUSeconds());

    // // Calculate expected event packet size
    // U32 minEventSize = sizeof(FwPacketDescriptorType) + sizeof(FwEventIdType) + Fw::Time::SERIALIZED_SIZE +
    //                    sizeof(I32);  // Severity as enum

    // printf("\nMinimum event packet size: %u bytes\n", minEventSize);
    // printf("================================\n\n");

    // printf("\n=== Endianness Check ===\n");

    // // Endianness check

    // uint32_t test = 0x01020304;
    // uint8_t* bytes = (uint8_t*)&test;

    // printf("Endianness check:\n");
    // printf("uint32_t value: 0x%08X\n", test);
    // printf("Byte order: %02X %02X %02X %02X\n", bytes[0], bytes[1], bytes[2], bytes[3]);

    // if (bytes[0] == 0x01) {
    //     printf("System is BIG-ENDIAN (SPARC expected)\n");
    // } else if (bytes[0] == 0x04) {
    //     printf("System is LITTLE-ENDIAN (unexpected for SPARC!)\n");
    // }
    printf("FwTimeBaseStoreType size: %d bytes\n", sizeof(FwTimeBaseStoreType));
    printf("FwTimeContextStoreType size: %d bytes\n", sizeof(FwTimeContextStoreType));


    // printf("========================\n\n");

    // Critical: Don't exit the loop until explicitly told to
    while (keep_running) {
        counter++;

        // Generate a tick every second (approximately)
        if (counter % 10 == 0) {
            // printf("[MAIN] Calling LedBlinker::checkAndProcessTimerTick()\n");
            LedBlinker::checkAndProcessTimerTick();
        }

        // Sleep for a short period
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() / 10);
    }

    // If we somehow exit the loop, don't exit immediately
    printf("Main loop exited, suspending main task\n");
    rtems_task_suspend(RTEMS_SELF);  // Keep the task alive

    // We should never reach here
    printf("Tearing down topology");
    LedBlinker::teardownTopology(inputs);
    printf("Topology teardown complete");

    return 0;
}
