// Add to a utility header (e.g., Os/RTEMS/QueueHelpers.hpp)
namespace Os {
namespace RTEMS {
// Os/RTEMS/Queue.cpp - Comprehensive fix
#include <errno.h>
#include <malloc.h>  // for memalign
#include <rtems.h>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Delegate.hpp>
#include <Os/Queue.hpp>
#include <Os/RTEMS/Queue.hpp>
#include <cstdint>  // for uintptr_t
#include <cstdlib>  // for free
#include <cstring>
/**
 * Safely receive a message from a queue, handling size mismatches
 * @return true if successful, false if error (including SIZE_MISMATCH)
 */
template <typename T>
bool safeQueueReceive(Os::Queue& queue, T& data) {
    FwSizeType actualSize = 0;
    FwQueuePriorityType priority = 0;

    // Ensure adequate buffer size
    if (sizeof(T) < queue.getMinBufferSize()) {
        Fw::Logger::logMsg("WARNING: Buffer too small for queue: %d < %d\n", sizeof(T), queue.getMinBufferSize());
        return false;
    }

    Queue::QueueStatus status =
        queue.receive(reinterpret_cast<U8*>(&data), sizeof(T), Queue::QUEUE_BLOCKING, actualSize, priority);

    return (status == Queue::QUEUE_OK);
}

}  // namespace RTEMS
}  // namespace Os