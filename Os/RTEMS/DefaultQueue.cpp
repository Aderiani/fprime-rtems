// ======================================================================
// \title Os/RTEMS/DefaultQueue.cpp
// \brief sets default Os::Queue to RTEMS implementation via linker
// ======================================================================
#include <Os/Queue.hpp>
#include "Os/RTEMS/Queue.hpp"
#include "Os/Delegate.hpp"

namespace Os {
QueueInterface* QueueInterface::getDelegate(QueueHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<QueueInterface, Os::RTEMS::Queue::RTEMSQueue>(aligned_new_memory);
}
}