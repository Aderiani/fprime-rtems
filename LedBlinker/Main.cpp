// ======================================================================
// \title  Main.cpp
// \brief Main program for F' application
// ======================================================================

#include <Fw/Logger/Logger.hpp>
#include <LedBlinker/Top/LedBlinkerTopology.hpp>
#include <Os/Os.hpp>
#include <cstdlib>  // For atoi
#include <cstring>  // For strcmp
#include <signal.h>
#ifdef __rtems__
#include <rtems.h>
// Add this for direct console output that doesn't rely on OS services
#include <stdio.h>
#define DEBUG_PRINT(msg) printf("%s\n", msg)
#else
#define DEBUG_PRINT(msg) /* empty in non-RTEMS builds */
#endif

// External network initialization functions (optional, only for RTEMS)
#ifdef __rtems__
extern "C" {
void system_init();
}
#endif

// Logging wrapper to handle different Logger interfaces
namespace {
void safeLogAdd(const char* message) {
    // For RTEMS, use direct printf for initialization debugging
#ifdef __rtems__
    printf("[F'] %s\n", message);
#else
    Fw::Logger::log(message);
#endif
}
}  // namespace
extern "C" {
#include "RTEMSInit/network_init.h"
}

static void signalHandler(int signum) {
    LedBlinker::stopSimulatedCycle();
}

extern "C" int fprime_main(int argc, char* argv[]) {
    // Prepare network configuration
    // struct FPrimeNetworkConfig network_config = default_network_config;

    // // Parse command-line arguments
    // for (int i = 1; i < argc; ++i) {
    //     if (strcmp(argv[i], "--no-dhcp") == 0) {
    //         network_config.use_dhcp = 0;
    //     } else if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
    //         network_config.use_dhcp = 0;
    //         network_config.static_ip = argv[++i];
    //     } else if (strcmp(argv[i], "--netmask") == 0 && i + 1 < argc) {
    //         network_config.netmask = argv[++i];
    //     } else if (strcmp(argv[i], "--gateway") == 0 && i + 1 < argc) {
    //         network_config.gateway = argv[++i];
    //     } else if (strcmp(argv[i], "--hostname") == 0 && i + 1 < argc) {
    //         network_config.hostname = argv[++i];
    //     }
    // }

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

    // Logging initialization
    safeLogAdd("Starting F' Application");

    // Setup topology
    DEBUG_PRINT("Setting up topology");
    LedBlinker::setupTopology(inputs);
    DEBUG_PRINT("Topology setup complete");

    // Start simulated cycle
    DEBUG_PRINT("Starting simulated cycle");
    LedBlinker::startSimulatedCycle(Fw::TimeInterval(1, 0));
    DEBUG_PRINT("Simulated cycle started");

    // Setup program shutdown via Ctrl-C
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    (void)printf("Hit Ctrl-C to quit\n");

    // Teardown topology
    DEBUG_PRINT("Tearing down topology");
    LedBlinker::teardownTopology(inputs);
    DEBUG_PRINT("Topology teardown complete");
    printf("Exiting fprime_main successfully\n");
    return 0;
}
