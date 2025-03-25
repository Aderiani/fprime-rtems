// ======================================================================
// \title  Main.cpp
// \brief Main program for F' application
// ======================================================================

#include <LedBlinker/Top/LedBlinkerTopology.hpp>
#include <Os/Os.hpp>
#include <Fw/Logger/Logger.hpp>
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
    extern NetworkConfig* create_network_config(
        int use_dhcp, 
        const char* static_ip, 
        const char* netmask, 
        const char* gateway
    );
    extern void cleanup_network_config(NetworkConfig* config);
}
#endif

// Logging wrapper to handle different Logger interfaces
namespace {
    void safeLogAdd(const char* message) {
        Fw::Logger::log(Fw::Logger::LogSeverity::INFO_LOW, message);
    }
}

// External function for setting up the topology
extern "C" int fprime_main(int argc, char* argv[]) {
    // Initialize OSAL
    Os::init();

    // Default configuration
    int use_dhcp = 1;
    const char* static_ip = nullptr;
    const char* netmask = nullptr;
    const char* gateway = nullptr;
    U16 port = 50000;  // Default port

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-dhcp") == 0) {
            use_dhcp = 0;
        } else if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
            static_ip = argv[++i];
        } else if (strcmp(argv[i], "--netmask") == 0 && i + 1 < argc) {
            netmask = argv[++i];
        } else if (strcmp(argv[i], "--gateway") == 0 && i + 1 < argc) {
            gateway = argv[++i];
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = static_cast<U16>(std::atoi(argv[++i]));
        }
    }

    // Network initialization (RTEMS-specific)
    #ifdef __rtems__
    NetworkConfig* net_config = create_network_config(
        use_dhcp, 
        static_ip, 
        netmask, 
        gateway
    );

    if (!net_config) {
        safeLogAdd("Failed to create network configuration");
        return -1;
    }

    if (initialize_network(net_config) != 0) {
        safeLogAdd("Network initialization failed");
        cleanup_network_config(net_config);
        return -1;
    }

    cleanup_network_config(net_config);
    #endif

    // Object for communicating state to the topology
    LedBlinker::TopologyState inputs;
    inputs.hostname = static_ip ? static_ip : "0.0.0.0";
    inputs.port = port;

    // Logging initialization
    safeLogAdd("Starting F' Application");

    // Setup topology
    LedBlinker::setupTopology(inputs);

    // Start simulated cycle
    LedBlinker::startSimulatedCycle(Fw::TimeInterval(1, 0));

    // Platform-specific run mechanism
    #ifdef __rtems__
    // RTEMS-specific delay
    rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 60);
    #else
    // Generic time-based delay for non-RTEMS platforms
    Os::Task::delay(Fw::TimeInterval(60, 0));
    #endif

    // Stop simulated cycle
    LedBlinker::stopSimulatedCycle();

    // Teardown topology
    LedBlinker::teardownTopology(inputs);

    return 0;
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