#include <signal.h>
#include <cstdlib>
#include <Fw/Logger/Logger.hpp>
#include <LedBlinker/Top/LedBlinkerTopology.hpp>
#include <Os/Os.hpp>
#include <rtems.h>
#include <rtems/rtems/clock.h>
#include <stdio.h>

extern "C" {
#include "RTEMSInit/network_init.h"
void initialize_rtems_clock(void);
}

static void signalHandler(int signum) {
    Fw::Logger::log("Main: Received signal %d, initiating shutdown\n", signum);
    LedBlinker::TopologyState inputs;
    inputs.hostname = "192.168.0.67";
    inputs.port = 50000;
    LedBlinker::teardownTopology(inputs);
    exit(0);
}

extern "C" int fprime_main(int argc, char* argv[]) {
    // Initialize OS
    Os::init();
    Fw::Logger::log("Main: OS initialized\n");

    // Initialize RTEMS clock
    initialize_rtems_clock();
    Fw::Logger::log("Main: RTEMS clock initialized\n");

    // Initialize network
    if (initialize_fprime_network() != 0) {
        Fw::Logger::log("Main: Failed to initialize network\n");
        return -1;
    }
    Fw::Logger::log("Main: Network initialized\n");

    // Setup topology
    LedBlinker::TopologyState inputs;
    inputs.hostname = "192.168.0.67";
    inputs.port = 50000;

    LedBlinker::setupTopology(inputs);
    Fw::Logger::log("Main: Topology setup complete\n");

    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    Fw::Logger::log("Main: Signal handlers installed, hit Ctrl-C to quit\n");

    // Diagnostic loop to verify system health
    volatile bool keep_running = true;
    unsigned int counter = 0;
    while (keep_running && counter < 50) { // Run for ~5s
        Fw::Logger::log("Main: Heartbeat %u, checking system health\n", counter);
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() / 10); // 100ms
        counter++;
    }

    // Suspend main task
    Fw::Logger::log("Main: Suspending main task, system running\n");
    rtems_task_suspend(RTEMS_SELF);

    // Should never reach here
    Fw::Logger::log("Main: Tearing down topology\n");
    LedBlinker::teardownTopology(inputs);
    Fw::Logger::log("Main: Topology teardown complete\n");
    return 0;
}