// ======================================================================
// \title Os/Posix/DefaultQueue.cpp
// \brief sets default Os::Queue to posix implementation via linker
// ======================================================================
#include "Os/Delegate.hpp"
#include "Os/Posix/Queue.hpp"
#include "Os/Queue.hpp"

namespace Os {
QueueInterface* QueueInterface::getDelegate(QueueHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<QueueInterface, Os::Posix::Queue::PosixQueue>(aligned_new_memory);
}
}  // namespace Os
