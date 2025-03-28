// ======================================================================
// \title  Main.cpp
// \brief Main program for F' application
// ======================================================================

#include <Fw/Logger/Logger.hpp>
#include <LedBlinker/Top/LedBlinkerTopology.hpp>
#include <Os/Os.hpp>
#include <cstdlib>  // For atoi
#include <cstring>  // For strcmp

#ifdef __rtems__
#include <rtems.h>
#endif

// External network initialization functions (optional, only for RTEMS)
#ifdef __rtems__
extern "C" {
typedef struct {
    int use_dhcp;
    const char* static_ip;
    const char* netmask;
    const char* gateway;
} NetworkConfig;

extern int initialize_network(NetworkConfig* config);
extern NetworkConfig* create_network_config(int use_dhcp,
                                            const char* static_ip,
                                            const char* netmask,
                                            const char* gateway);
extern void cleanup_network_config(NetworkConfig* config);
}
#endif

// Logging wrapper to handle different Logger interfaces
namespace {
void safeLogAdd(const char* message) {
    Fw::Logger::log(message);
}
}  // namespace

// External function for setting up the topology

extern "C" int fprime_main(int argc, char* argv[]) {
    printf("F' main starting...\n");
    
    // Initialize OSAL
    printf("Initializing OS...\n");
    Os::init();
    printf("OS initialized successfully\n");
    
    // Setup topology
    printf("Setting up topology...\n");
    LedBlinker::TopologyState inputs;
    inputs.hostname = "0.0.0.0";
    inputs.port = 50000;
    
    try {
        LedBlinker::setupTopology(inputs);
        printf("Topology setup complete\n");
        
        printf("Starting simulated cycle...\n");
        LedBlinker::startSimulatedCycle(Fw::TimeInterval(1, 0));
        printf("Simulated cycle started\n");
        
        printf("Running for 10 seconds...\n");
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 10);
        
        printf("Stopping simulated cycle...\n");
        LedBlinker::stopSimulatedCycle();
        printf("Simulated cycle stopped\n");
        
        printf("Tearing down topology...\n");
        LedBlinker::teardownTopology(inputs);
        printf("Topology torn down\n");
        
        printf("F' application completed successfully\n");
        return 0;
    } catch (const std::exception& e) {
        printf("Exception during F' execution: %s\n", e.what());
        return -1;
    } catch (...) {
        printf("Unknown exception during F' execution\n");
        return -1;
    }
}
// RTEMS requires a special main for C++ applications
#ifdef __rtems__
extern "C" int main(int argc, char* argv[]) {
    return fprime_main(argc, argv);
}
#else
// Standard main for non-RTEMS platforms
int main(int argc, char* argv[]) {
    return fprime_main(argc, argv);
}
#endif