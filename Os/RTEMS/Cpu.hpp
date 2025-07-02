// ======================================================================
// \title Os/RTEMS/Cpu.hpp
// \brief RTEMS implementation for Os::Cpu with L4STAT support
// ======================================================================
#ifndef OS_RTEMS_CPU_HPP
#define OS_RTEMS_CPU_HPP

#include <Os/Cpu.hpp>
#include <rtems.h>

// Maximum processors configuration
#ifndef CONFIGURE_MAXIMUM_PROCESSORS
#define CONFIGURE_MAXIMUM_PROCESSORS 4
#endif

namespace Os {
namespace RTEMS {
namespace Cpu {

//! RtemsCpuHandle class definition for RTEMS implementations
struct RtemsCpuHandle : public CpuHandle {};

//! \brief RTEMS implementation of CpuInterface using L4STAT hardware counters
//!
//! This implementation uses the GR740's L4STAT hardware performance counters
//! to accurately measure CPU usage by tracking execution cycles vs hold cycles.
//!
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
    //! \param cpu_count: output parameter for number of CPUs
    //! \return: OP_OK on success, ERROR on failure
    Status _getCount(FwSizeType& cpu_count) override;

    //! Get CPU ticks for a specific CPU using L4STAT counters
    //! \param ticks: output parameter for CPU usage (used/total)
    //! \param cpu_index: which CPU to query (0-based)
    //! \return: OP_OK on success, ERROR on failure
    Status _getTicks(Os::Cpu::Ticks& ticks, FwSizeType cpu_index) override;

    //! Get CPU handle
    //! \return: pointer to the CPU handle
    CpuHandle* getHandle() override;

  private:
    RtemsCpuHandle m_handle;
};

//! \brief Cleanup L4STAT counters (optional utility function)
//!
//! This function can be called to properly disable L4STAT counters
//! when CPU monitoring is no longer needed.
void cleanup_l4stat_counters();

} // namespace Cpu
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_CPU_HPP