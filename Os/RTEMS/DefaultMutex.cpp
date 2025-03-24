// ======================================================================
// \title Os/RTEMS/DefaultMutex.cpp
// \brief sets default Os::Mutex to RTEMS implementation via linker
// ======================================================================
#include "Os/Mutex.hpp"
#include "Os/RTEMS/Mutex.hpp"
#include "Os/Delegate.hpp"

namespace Os {
MutexInterface* MutexInterface::getDelegate(MutexHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MutexInterface, Os::RTEMS::Mutex::RTEMSMutex>(aligned_new_memory);
}
}