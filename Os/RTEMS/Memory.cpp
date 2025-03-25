// ======================================================================
// \title Os/RTEMS/Memory.cpp
// \brief RTEMS implementation for Os::Memory
// ======================================================================
#include "Os/RTEMS/Memory.hpp"
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <stdlib.h>

namespace Os {
namespace RTEMS {
namespace Memory {

MemoryInterface::Status RtemsMemory::_getUsage(Os::Memory::Usage& memory_usage) {
    // Attempt to get memory information using RTEMS/system methods
    struct mallinfo mem_info = mallinfo();
    
    // Calculate total and used memory
    // mallinfo provides information about the memory allocated by malloc
    memory_usage.total = static_cast<FwSizeType>(mem_info.arena);       // Total space allocated by malloc
    memory_usage.used = static_cast<FwSizeType>(mem_info.uordblks);     // Total space used

    // Sanity check
    if (memory_usage.used > memory_usage.total) {
        return MemoryInterface::Status::ERROR;
    }
    
    return MemoryInterface::Status::OP_OK;
}

MemoryHandle* RtemsMemory::getHandle() {
    return &m_handle;
}

} // namespace Memory
} // namespace RTEMS
} // namespace Os
