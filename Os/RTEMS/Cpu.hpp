// ======================================================================
// \title Os/RTEMS/Cpu.hpp
// \brief RTEMS implementation for Os::Cpu
// ======================================================================
#ifndef OS_RTEMS_CPU_HPP
#define OS_RTEMS_CPU_HPP

#include <Os/Cpu.hpp>
#include <rtems.h>
#include <rtems/rtems/tasks.h>

namespace Os {
namespace RTEMS {
namespace Cpu {

//! RtemsCpuHandle class definition for RTEMS implementations
struct RtemsCpuHandle : public CpuHandle {};

//! \brief RTEMS implementation of CpuInterface
class RtemsCpu : public CpuInterface {
  public:
    //! Constructor
    RtemsCpu() = default;

    //! Destructor
    ~RtemsCpu() override = default;

    //! Get CPU count
    Status _getCount(FwSizeType& cpu_count) override;

    //! Get CPU ticks for a specific CPU
    Status _getTicks(Os::Cpu::Ticks& ticks, FwSizeType cpu_index) override;

    //! Get CPU handle
    CpuHandle* getHandle() override;

  private:
    RtemsCpuHandle m_handle;
};

} // namespace Cpu
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_CPU_HPP