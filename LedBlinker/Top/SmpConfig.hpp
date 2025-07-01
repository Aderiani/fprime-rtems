#ifndef SMP_CONFIG_HPP
#define SMP_CONFIG_HPP

#include <rtems.h>
#include <rtems/score/smp.h>  // For _SMP_Get_processor_maximum()

namespace LedBlinker {

// CPU assignment strategy for F Prime components
struct SmpConfig {
    static constexpr int CPU_COMMAND = 0;      // Command processing
    static constexpr int CPU_TELEMETRY = 1;    // Telemetry and logging
    static constexpr int CPU_DRIVERS = 2;      // Hardware drivers
    static constexpr int CPU_APPLICATION = 3;  // Application logic
    
    // Helper to distribute components across CPUs
    static void configureComponentAffinity() {
        // This will be called after topology setup
        printf("Configuring SMP affinity for F Prime components...\n");
        
        // Use the new API to get processor count
        #ifdef RTEMS_SMP
        uint32_t cpu_count = _SMP_Get_processor_maximum();
        #else
        uint32_t cpu_count = 1;
        #endif
        
        printf("System has %d CPUs available\n", cpu_count);
        
        // TODO: Set affinity for each component's thread
        // This will be implemented when we know which components you're using
    }
};

} // namespace LedBlinker

#endif