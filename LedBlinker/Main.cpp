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
// Add this for direct console output that doesn't rely on OS services
#include <stdio.h>
#define DEBUG_PRINT(msg) printf("%s\n", msg)
#else
#define DEBUG_PRINT(msg) /* empty in non-RTEMS builds */
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
// namespace {
// void safeLogAdd(const char* message) {
//     // For RTEMS, use direct printf for initialization debugging
// #ifdef __rtems__
//     printf("[F'] %s\n", message);
// #else
//     Fw::Logger::log(message);
// #endif
// }
// }  // namespace

// External function for setting up the topology
// extern "C" int fprime_main(int argc, char* argv[]) {
//     DEBUG_PRINT("fprime_main started");
    
//     // Initialize OSAL - THIS IS WHERE IT'S HANGING
//     DEBUG_PRINT("Calling Os::init()");
//     Os::init();
//     DEBUG_PRINT("Os::init() completed");

//     // Default configuration
//     int use_dhcp = 0;                        // Set to 0 to use static IP
//     const char* static_ip = "192.168.1.10";  // Network's static IP
//     const char* netmask = "255.255.255.0";
//     const char* gateway = "192.168.1.1";  
//     U16 port = 50000;                     // Default port

//     // Parse command-line arguments
//     for (int i = 1; i < argc; ++i) {
//         if (strcmp(argv[i], "--no-dhcp") == 0) {
//             use_dhcp = 0;
//         } else if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
//             static_ip = argv[++i];
//         } else if (strcmp(argv[i], "--netmask") == 0 && i + 1 < argc) {
//             netmask = argv[++i];
//         } else if (strcmp(argv[i], "--gateway") == 0 && i + 1 < argc) {
//             gateway = argv[++i];
//         } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
//             port = static_cast<U16>(std::atoi(argv[++i]));
//         }
//     }

// // Network initialization (RTEMS-specific)
// #ifdef __rtems__
//     DEBUG_PRINT("Creating network config");
//     NetworkConfig* net_config = create_network_config(use_dhcp, static_ip, netmask, gateway);

//     if (!net_config) {
//         safeLogAdd("Failed to create network configuration");
//         return -1;
//     }

//     DEBUG_PRINT("Initializing network");
//     if (initialize_network(net_config) != 0) {
//         safeLogAdd("Network initialization failed");
//         cleanup_network_config(net_config);
//         return -1;
//     }

//     cleanup_network_config(net_config);
//     DEBUG_PRINT("Network initialized");
// #endif

//     // Object for communicating state to the topology
//     LedBlinker::TopologyState inputs;
//     inputs.hostname = static_ip ? static_ip : "0.0.0.0";
//     inputs.port = port;

//     // Logging initialization
//     safeLogAdd("Starting F' Application");

//     // Setup topology
//     DEBUG_PRINT("Setting up topology");
//     LedBlinker::setupTopology(inputs);
//     DEBUG_PRINT("Topology setup complete");

//     // Start simulated cycle
//     DEBUG_PRINT("Starting simulated cycle");
//     LedBlinker::startSimulatedCycle(Fw::TimeInterval(1, 0));
//     DEBUG_PRINT("Simulated cycle started");

// // Platform-specific run mechanism
// #ifdef __rtems__
//     // RTEMS-specific delay
//     DEBUG_PRINT("Delaying for 60 seconds");
//     rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 60);
// #else
//     // Generic time-based delay for non-RTEMS platforms
//     Os::Task::delay(Fw::TimeInterval(60, 0));
// #endif

//     // Stop simulated cycle
//     DEBUG_PRINT("Stopping simulated cycle");
//     LedBlinker::stopSimulatedCycle();
//     DEBUG_PRINT("Simulated cycle stopped");

//     // Teardown topology
//     DEBUG_PRINT("Tearing down topology");
//     LedBlinker::teardownTopology(inputs);
//     DEBUG_PRINT("Topology teardown complete");
//     printf("Exiting fprime_main successfully\n");
//     return 0;
// }

// RTEMS requires a special main for C++ applications
// #ifdef __rtems__
// extern "C" int main(int argc, char* argv[]) {
//     printf("RTEMS main() starting...\n");
//     int result = fprime_main(argc, argv);
//     printf("RTEMS main() completed with result: %d\n", result);
//     return result;
// }
// #else
// // Standard main for non-RTEMS platforms
// int main(int argc, char* argv[]) {
//     return fprime_main(argc, argv);
// }
// #endif
// RTEMS requires a special main for C++ applications
// RTEMS requires a special main for C++ applications


// Modify the fprime_main function to also not return normally
extern "C" int fprime_main(int argc, char* argv[]) {
    printf("fprime_main started\n");
    
    // Initialize OSAL - modified to do nothing for RTEMS
    printf("Calling Os::init()\n");
    Os::init();
    printf("Os::init() completed\n");
    
    // Add more F Prime operations here as needed
    
    printf("fprime_main operations completed\n");
    
#ifdef __rtems__
    // For RTEMS, don't return normally
    printf("fprime_main returning control to main\n");
    return 0;
#else
    // For other platforms, return normally
    return 0;
#endif
}



#ifdef __rtems__
extern "C" int main(int argc, char* argv[]) {
    printf("RTEMS main() starting...\n");
    
    // Call fprime_main but don't use its return value
    fprime_main(argc, argv);
    
    printf("RTEMS main() entering permanent idle state\n");
    
    // Force the application to remain running indefinitely
    // This avoids destructors and C++ runtime cleanup that may be causing issues
    for(;;) {
        // Sleep for 1 second
        rtems_task_wake_after(rtems_clock_get_ticks_per_second());
    }
    
    // This line will never be reached
    return 0;
}
#else
// Standard main for non-RTEMS platforms
int main(int argc, char* argv[]) {
    return fprime_main(argc, argv);
}
#endif

