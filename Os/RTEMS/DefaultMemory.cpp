// ======================================================================
// \title Os/RTEMS/DefaultMemory.cpp
// \brief sets default Os::Memory to RTEMS implementation via linker
// ======================================================================
#include "Os/Memory.hpp"
#include "Os/RTEMS/Memory.hpp"
#include "Os/Delegate.hpp"

namespace Os {
MemoryInterface* MemoryInterface::getDelegate(MemoryHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MemoryInterface, Os::RTEMS::Memory::RtemsMemory>(aligned_new_memory);
}
}