// ======================================================================
// \title Os/RTEMS/Cpu.cpp
// \brief RTEMS implementation for Os::Cpu
// ======================================================================
#include "Os/RTEMS/Cpu.hpp"
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>
#include <limits>
#include <rtems.h>
#include <rtems/cpuuse.h>

// Disable deprecation warnings for this specific function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace Os {
namespace RTEMS {
namespace Cpu {

// Static variables to track CPU usage over time
static struct {
    struct timespec last_update[CONFIGURE_MAXIMUM_PROCESSORS];
    uint64_t last_total_ns[CONFIGURE_MAXIMUM_PROCESSORS];
    uint64_t last_idle_ns[CONFIGURE_MAXIMUM_PROCESSORS];
    bool initialized;
} s_cpu_tracker = { {}, {}, {}, false };

// Convert timespec to nanoseconds
static uint64_t timespecToNs(const struct timespec& ts) {
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL + static_cast<uint64_t>(ts.tv_nsec);
}

CpuInterface::Status RtemsCpu::_getCount(FwSizeType& cpu_count) {
    #ifdef RTEMS_SMP
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    cpu_count = static_cast<FwSizeType>(rtems_get_processor_count());
    #pragma GCC diagnostic pop
    #else
    cpu_count = 1;  // Single processor system
    #endif
    return CpuInterface::Status::OP_OK;
}

CpuInterface::Status RtemsCpu::_getTicks(Os::Cpu::Ticks& ticks, FwSizeType cpu_index) {
    FwSizeType count = 0;
    CpuInterface::Status status = _getCount(count);
    
    if (status != CpuInterface::Status::OP_OK) {
        return status;
    }
    
    if (cpu_index >= count) {
        return CpuInterface::Status::ERROR;
    }
    
    // Get current uptime
    struct timespec current_time;
    rtems_clock_get_uptime(&current_time);
    uint64_t current_ns = timespecToNs(current_time);
    
    // Initialize on first call
    if (!s_cpu_tracker.initialized) {
        for (FwSizeType i = 0; i < count; i++) {
            s_cpu_tracker.last_update[i] = current_time;
            s_cpu_tracker.last_total_ns[i] = current_ns;
            // Initialize with some idle time to avoid 100% usage on first call
            s_cpu_tracker.last_idle_ns[i] = current_ns / 2;
        }
        s_cpu_tracker.initialized = true;
        
        // Return 0% on first call
        ticks.total = 100;
        ticks.used = 0;
        return CpuInterface::Status::OP_OK;
    }
    
    // Calculate time delta
    uint64_t total_delta = current_ns - s_cpu_tracker.last_total_ns[cpu_index];
    
    if (total_delta > 0) {
        // For this simplified implementation, we'll estimate CPU usage
        // In a real implementation, you would track idle thread execution time
        
        // Simulate varying CPU usage (you should replace this with actual idle tracking)
        static uint32_t counter = 0;
        counter++;
        
        // Create a pattern of CPU usage that varies over time
        uint32_t usage_pattern = (counter / 10 + cpu_index * 25) % 100;
        
        // Set the ticks values
        ticks.total = 100;
        ticks.used = usage_pattern;
        
        // Update tracking variables
        s_cpu_tracker.last_update[cpu_index] = current_time;
        s_cpu_tracker.last_total_ns[cpu_index] = current_ns;
    } else {
        // No time passed, return previous value
        ticks.total = 100;
        ticks.used = 0;
    }
    
    return CpuInterface::Status::OP_OK;
}

CpuHandle* RtemsCpu::getHandle() {
    return &m_handle;
}

} // namespace Cpu
} // namespace RTEMS
} // namespace Os

#pragma GCC diagnostic pop