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

    printf("Entering F' main infinite loop\n");

    // checkResources();
    volatile bool keep_running = true;
    rtems_id task_id;
    rtems_task_ident(RTEMS_SELF, RTEMS_SEARCH_LOCAL_NODE, &task_id);
    printf("Main task ID: %lu\n", (unsigned long)task_id);
    
    // In the main loop
    int counter = 0;
    printf("Entering enhanced main loop\n");
    while (keep_running) {
        rtems_task_ident(RTEMS_SELF, RTEMS_SEARCH_LOCAL_NODE, &task_id);
        printf("Loop #%u - Task ID: %lu\n", counter, (unsigned long)task_id);
        
        // Normal heartbeat code
        if (counter % 10 == 0) {
            printf("F' style main heartbeat2: %u\n", counter/10);
        }
        counter++;
        
        // Force a flush to ensure output is seen
        fflush(stdout);
        
        // Add a barrier to prevent optimization
        asm volatile("" ::: "memory");
        
        // Very short delay to allow for more debug output
        rtems_task_wake_after(2);
    }

    // We should never reach here
    printf("Tearing down topology");
    LedBlinker::teardownTopology(inputs);
    printf("Topology teardown complete");

    return 0;
}
