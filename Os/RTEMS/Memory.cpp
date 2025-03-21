// ======================================================================
// \title Os/RTEMS/Memory.cpp
// \brief RTEMS implementation for Os::Memory
// ======================================================================
#include <Os/Memory.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <stdlib.h>
#include <malloc.h>

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
        // Attempt to get memory information using RTEMS/system methods
        struct mallinfo mem_info = mallinfo();
        
        // Calculate total and used memory
        // mallinfo provides information about the memory allocated by malloc
        memory_usage.total = static_cast<FwSizeType>(mem_info.arena);       // Total space allocated by malloc
        memory_usage.used = static_cast<FwSizeType>(mem_info.uordblks);     // Total space used

        // Sanity check
        if (memory_usage.used > memory_usage.total) {
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
    // The address cannot be null due to how it's allocated
    static_assert(sizeof(Os::RTEMS::Memory::RtemsMemory) <= sizeof(MemoryHandleStorage),
                  "RTEMS Memory implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Memory::RtemsMemory)) == 0,
                  "Bad alignment for RTEMS Memory implementation");
    return new (aligned_new_memory) Os::RTEMS::Memory::RtemsMemory;
}
}