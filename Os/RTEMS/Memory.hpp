// ======================================================================
// \title Os/RTEMS/Memory.hpp
// \brief RTEMS implementation for Os::Memory
// ======================================================================
#ifndef OS_RTEMS_MEMORY_HPP
#define OS_RTEMS_MEMORY_HPP

#include <Os/Memory.hpp>

namespace Os {
namespace RTEMS {
namespace Memory {

//! RtemsMemoryHandle class definition for RTEMS implementations
struct RtemsMemoryHandle : public MemoryHandle {};

//! \brief RTEMS implementation of MemoryInterface
class RtemsMemory : public MemoryInterface {
  public:
    //! Constructor
    RtemsMemory() = default;

    //! Destructor
    ~RtemsMemory() override = default;

    //! Copy constructor - deleted
    RtemsMemory(const RtemsMemory& other) = delete;

    //! Assignment operator - deleted
    RtemsMemory& operator=(const MemoryInterface& other) override = delete;

    //! Get memory usage
    Status _getUsage(Os::Memory::Usage& memory_usage) override;

    //! Get memory handle
    MemoryHandle* getHandle() override;

  private:
    RtemsMemoryHandle m_handle;
};

} // namespace Memory
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_MEMORY_HPP