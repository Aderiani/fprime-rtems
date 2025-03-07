// ======================================================================
// \title Os/RTEMS/Memory.cpp
// \brief RTEMS implementation for Os::Memory
// ======================================================================
#include <Os/Memory.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <rtems/malloc.h>

namespace Os {
namespace RTEMS {
namespace Memory {

struct RtemsMemoryHandle : public MemoryHandle {};

class RtemsMemory : public MemoryInterface {
  public:
    //! Constructor
    RtemsMemory() = default;

    //! Destructor
    ~RtemsMemory() override = default;

    //! Get memory usage
    Status _getUsage(Os::Memory::Usage& memory_usage) override {
        // Get memory statistics from RTEMS
        rtems_malloc_statistics_t stats;
        rtems_malloc_get_statistics(&stats);
        
        // Fill in memory usage values
        memory_usage.total = stats.space_available;
        
        // Calculate used memory - this is an approximation
        if (stats.space_available >= stats.free_size) {
            memory_usage.used = stats.space_available - stats.free_size;
        } else {
            // This shouldn't happen, but handle it gracefully
            memory_usage.used = 0;
            return Status::ERROR;
        }
        
        return Status::OP_OK;
    }

    //! Get memory handle
    MemoryHandle* getHandle() override {
        return &m_handle;
    }

  private:
    RtemsMemoryHandle m_handle;
};

} // namespace Memory
} // namespace RTEMS
} // namespace Os

namespace Os {
MemoryInterface* MemoryInterface::getDelegate(MemoryHandleStorage& aligned_new_memory) {
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::Memory::RtemsMemory) <= sizeof(MemoryHandleStorage),
                  "RTEMS Memory implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Memory::RtemsMemory)) == 0,
                  "Bad alignment for RTEMS Memory implementation");
    return new (aligned_new_memory) Os::RTEMS::Memory::RtemsMemory;
}
}