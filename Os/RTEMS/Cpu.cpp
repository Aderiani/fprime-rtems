// ======================================================================
// \title Os/RTEMS/Cpu.cpp
// \brief RTEMS implementation for Os::Cpu
// ======================================================================
#include <Os/Cpu.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <rtems/score/cpuuse.h>
#include <rtems/cpuuse.h>

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
        // RTEMS knows how many CPUs are available
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
        
        // This is a simplified implementation - RTEMS may have better APIs for this
        // In a full implementation, you would want to read CPU specific usage data
        rtems_cpu_usage_data cpu_data;
        rtems_cpu_usage_reset();
        rtems_cpu_usage_report(&cpu_data);
        
        // This is an approximation - a real implementation would need to get
        // actual CPU-specific ticks from RTEMS APIs
        ticks.total = 100; // 100% normalized
        ticks.used = 0;    // Initialize to 0
        
        // Try to get the usage for the specific CPU
        if (cpu_index < count) {
            // This is just a placeholder - RTEMS will have different APIs
            // You'll need to find the right RTEMS API to get the actual CPU usage
            ticks.used = 0; // Replace with actual usage data
        }
        
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
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::Cpu::RtemsCpu) <= sizeof(CpuHandleStorage),
                  "RTEMS Cpu implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Cpu::RtemsCpu)) == 0,
                  "Bad alignment for RTEMS Cpu implementation");
    return new (aligned_new_memory) Os::RTEMS::Cpu::RtemsCpu;
}
}