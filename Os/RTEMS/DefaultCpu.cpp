// ======================================================================
// \title Os/RTEMS/DefaultCpu.cpp
// \brief sets default Os::Cpu to RTEMS implementation via linker
// ======================================================================
#include "Os/Cpu.hpp"
#include "Os/RTEMS/Cpu.hpp"
#include "Os/Delegate.hpp"

namespace Os {
CpuInterface* CpuInterface::getDelegate(CpuHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<CpuInterface, Os::RTEMS::Cpu::RtemsCpu>(aligned_new_memory);
}
}
