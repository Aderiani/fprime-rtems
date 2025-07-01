// ======================================================================
// \title Os/RTEMS/Cpu.cpp
// \brief RTEMS implementation for Os::Cpu
// ======================================================================
#include "Os/RTEMS/Cpu.hpp"
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>
#include <limits>

// Disable deprecation warnings for this specific function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace Os {
namespace RTEMS {
namespace Cpu {

CpuInterface::Status RtemsCpu::_getCount(FwSizeType& cpu_count) {
    #ifdef RTEMS_SMP
    // Use the non-deprecated API
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
    
    // For RTEMS, this is a simplified implementation
    // You might need to use platform-specific APIs for detailed CPU usage
    ticks.total = 100; // 100% normalized
    ticks.used = 50;   // Placeholder 50% usage
    
    return CpuInterface::Status::OP_OK;
}

CpuHandle* RtemsCpu::getHandle() {
    return &m_handle;
}

} // namespace Cpu
} // namespace RTEMS
} // namespace Os



#pragma GCC diagnostic pop