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

// // In Main.cpp, at various points:
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

    // Initialize network
    if (initialize_fprime_network() != 0) {
        printf("Failed to initialize network\n");
        // return -1;
    }

    LedBlinker::TopologyState inputs;

    inputs.hostname = "192.168.0.67";
    inputs.port = 50000;  // Default port

    // Setup topology
    printf("Setting up topology");
    LedBlinker::setupTopology(inputs);
    printf("Topology setup complete");
    fflush(stdout);

    // // Setup program shutdown via Ctrl-C
    // signal(SIGINT, signalHandler);
    // signal(SIGTERM, signalHandler);
    // (void)printf("Hit Ctrl-C to quit\n");

    // In LedBlinker/Main.cpp main loop:
    volatile bool keep_running = true;
    unsigned int counter = 0;

    printf("Entering F' main infinite loop with periodic timer ticks\n");

    // Critical: Don't exit the loop until explicitly told to
    while (keep_running) {
        // Print heartbeat occasionally
        if (counter % 500 == 0) {
            printf("F' style main heartbeat: %u\n", counter / 500);
        }
        counter++;

        // Generate a tick every second (approximately)
        if (counter % 10 == 0) {  
            printf("[MAIN] Calling LedBlinker::checkAndProcessTimerTick()\n");
            LedBlinker::checkAndProcessTimerTick();
        }
        
        if (counter % 50 == 0) {  // Every ~5 seconds
            printf("F' style main heartbeat: %u\n", counter / 50);
            LedBlinker::forceTelemetryDownlink();
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
