// ======================================================================
// \title Os/RTEMS/DefaultRawTime.cpp
// \brief sets default Os::RawTime to RTEMS implementation via linker
// ======================================================================
#include "Os/RawTime.hpp"
#include "Os/RTEMS/RawTime.hpp"
#include "Os/Delegate.hpp"

namespace Os {
RawTimeInterface* RawTimeInterface::getDelegate(RawTimeHandleStorage& aligned_new_memory, const RawTimeInterface* to_copy) {
    // Ignore to_copy parameter if provided
    return Os::Delegate::makeDelegate<RawTimeInterface, Os::RTEMS::RawTime::RtemsRawTime>(aligned_new_memory);
}
}