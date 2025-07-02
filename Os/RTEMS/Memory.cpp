// ======================================================================
// \title Os/RTEMS/Memory.cpp
// \brief RTEMS implementation for Os::Memory
// ======================================================================
#include "Os/RTEMS/Memory.hpp"
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <rtems/config.h>
#include <stdlib.h>
#include <malloc.h>

namespace Os {
namespace RTEMS {
namespace Memory {

MemoryInterface::Status RtemsMemory::_getUsage(Os::Memory::Usage& memory_usage) {
    // Default to 150MB as configured in rtems_config.h
    const FwSizeType configured_memory = 150 * 1024 * 1024; // 150MB
    
    // Try to get heap usage from mallinfo
    struct mallinfo mi = mallinfo();
    
    if (mi.arena > 0) {
        // mallinfo provides heap information
        memory_usage.total = static_cast<FwSizeType>(mi.arena);       // Total heap size
        memory_usage.used = static_cast<FwSizeType>(mi.uordblks);     // Used heap size
    } else {
        // Fallback: Use configured memory size
        memory_usage.total = configured_memory;
        
        // Estimate used memory - start with a reasonable default
        // This could be improved by tracking allocations
        memory_usage.used = configured_memory / 10;  // Assume 10% used initially
    }
    
    // Ensure values make sense
    if (memory_usage.total == 0) {
        memory_usage.total = configured_memory;
    }
    
    if (memory_usage.used > memory_usage.total) {
        memory_usage.used = memory_usage.total;
    }
    
    return MemoryInterface::Status::OP_OK;
}

MemoryHandle* RtemsMemory::getHandle() {
    return &m_handle;
}

} // namespace Memory
} // namespace RTEMS
} // namespace Os