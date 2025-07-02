// ======================================================================
// \title Os/RTEMS/Cpu.hpp
// \brief RTEMS implementation for Os::Cpu
// ======================================================================
#ifndef OS_RTEMS_CPU_HPP
#define OS_RTEMS_CPU_HPP

#include <Os/Cpu.hpp>
#include <rtems.h>
#include <rtems/rtems/tasks.h>
#include <rtems/score/timestamp.h>

// Include the maximum processors configuration
#ifndef CONFIGURE_MAXIMUM_PROCESSORS
#define CONFIGURE_MAXIMUM_PROCESSORS 4
#endif

// Define CPU usage data structure if not available in RTEMS
#ifndef RTEMS_CPU_USAGE_DATA_DEFINED
typedef struct {
    Timestamp_Control total_elapsed_time;
    Timestamp_Control idle_elapsed_time;
} rtems_cpu_usage_data;
#endif

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

    //! Copy constructor - deleted
    RtemsCpu(const RtemsCpu& other) = delete;

    //! Assignment operator - deleted
    RtemsCpu& operator=(const CpuInterface& other) override = delete;

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