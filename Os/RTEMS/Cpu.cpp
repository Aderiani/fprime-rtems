// ======================================================================
// \title Os/RTEMS/Cpu.cpp
// \brief RTEMS implementation for Os::Cpu
// ======================================================================
#include <Os/Cpu.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <rtems/rtems/tasks.h>

// Disable deprecation warnings for this specific function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace Os {
namespace RTEMS {
namespace Cpu {

struct RtemsCpuHandle : public CpuHandle {};

class RtemsCpu : public CpuInterface {
  public:
    //! Constructor
    RtemsCpu() = default;

    //! Destructor
    ~RtemsCpu() override = default;

    //! Get CPU count
    Status _getCount(FwSizeType& cpu_count) override {
        // Use rtems_get_processor_count(), suppressing deprecation warning
        cpu_count = static_cast<FwSizeType>(rtems_get_processor_count());
        return Status::OP_OK;
    }

    //! Get CPU ticks for a specific CPU
    Status _getTicks(Os::Cpu::Ticks& ticks, FwSizeType cpu_index) override {
        FwSizeType count = 0;
        Status status = _getCount(count);
        
        if (status != Status::OP_OK) {
            return status;
        }
        
        if (cpu_index >= count) {
            return Status::ERROR;
        }
        
        // For RTEMS, this is a simplified implementation
        // You might need to use platform-specific APIs for detailed CPU usage
        ticks.total = 100; // 100% normalized
        ticks.used = 50;   // Placeholder 50% usage
        
        return Status::OP_OK;
    }

    //! Get CPU handle
    CpuHandle* getHandle() override {
        return &m_handle;
    }

  private:
    RtemsCpuHandle m_handle;
};

} // namespace Cpu
} // namespace RTEMS
} // namespace Os

namespace Os {
CpuInterface* CpuInterface::getDelegate(CpuHandleStorage& aligned_new_memory) {
    // The address cannot be null due to how it's allocated
    static_assert(sizeof(Os::RTEMS::Cpu::RtemsCpu) <= sizeof(CpuHandleStorage),
                  "RTEMS Cpu implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Cpu::RtemsCpu)) == 0,
                  "Bad alignment for RTEMS Cpu implementation");
    return new (aligned_new_memory) Os::RTEMS::Cpu::RtemsCpu;
}
}

#pragma GCC diagnostic pop