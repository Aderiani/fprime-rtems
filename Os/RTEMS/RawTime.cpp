// ======================================================================
// \title Os/RTEMS/RawTime.cpp
// \brief RTEMS implementation for Os::RawTime
// ======================================================================
#include "Os/RTEMS/RawTime.hpp"
#include <Fw/Types/Assert.hpp>
#include <limits>

namespace Os {
namespace RTEMS {
namespace RawTime {

RtemsRawTime::RtemsRawTime() {
    m_handle.time.tv_sec = 0;
    m_handle.time.tv_nsec = 0;
}

RawTimeInterface::Status RtemsRawTime::now() {
    if (clock_gettime(CLOCK_REALTIME, &m_handle.time) != 0) {
        return Status::OTHER_ERROR;
    }
    return RawTimeInterface::Status::OP_OK;
}

RawTimeInterface::Status RtemsRawTime::getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const {
    // We'll use a conservative approach that assumes basic serialization
    U32 otherSec = 0, otherNsec = 0;
    
    // Attempt to serialize the other time object
    Fw::SerializeBufferBase* serBuffer = nullptr;
    Fw::SerializeStatus serStatus = other.serialize(*serBuffer);
    
    if (serStatus != Fw::FW_SERIALIZE_OK) {
        return RawTimeInterface::Status::OTHER_ERROR;
    }
    
    // Deserialize seconds and nanoseconds
    serStatus = serBuffer->deserialize(otherSec);
    if (serStatus != Fw::FW_SERIALIZE_OK) {
        return RawTimeInterface::Status::OTHER_ERROR;
    }

    serStatus = serBuffer->deserialize(otherNsec);
    if (serStatus != Fw::FW_SERIALIZE_OK) {
        return RawTimeInterface::Status::OTHER_ERROR;
    }

    // Calculate difference in seconds and nanoseconds
    time_t sec_diff = m_handle.time.tv_sec - static_cast<time_t>(otherSec);
    long nsec_diff = m_handle.time.tv_nsec - static_cast<long>(otherNsec);
    
    // Adjust for negative nanoseconds
    if (nsec_diff < 0) {
        sec_diff--;
        nsec_diff += 1000000000L;
    }
    
    // Check for overflow
    if (sec_diff > static_cast<time_t>(std::numeric_limits<U32>::max())) {
        return Status::OP_OVERFLOW;
    }
    
    interval.set(static_cast<U32>(sec_diff), static_cast<U32>(nsec_diff));
    return Status::OP_OK;
}

Fw::SerializeStatus RtemsRawTime::serialize(Fw::SerializeBufferBase& buffer) const {
    // Serialize seconds and nanoseconds
    Fw::SerializeStatus status;
    
    status = buffer.serialize(static_cast<U32>(m_handle.time.tv_sec));
    if (status != Fw::FW_SERIALIZE_OK) {
        return status;
    }
    
    status = buffer.serialize(static_cast<U32>(m_handle.time.tv_nsec));
    return status;
}

Fw::SerializeStatus RtemsRawTime::deserialize(Fw::SerializeBufferBase& buffer) {
    // Deserialize seconds and nanoseconds
    U32 seconds = 0;
    U32 nanoseconds = 0;
    Fw::SerializeStatus status;
    
    status = buffer.deserialize(seconds);
    if (status != Fw::FW_SERIALIZE_OK) {
        return status;
    }
    
    status = buffer.deserialize(nanoseconds);
    if (status != Fw::FW_SERIALIZE_OK) {
        return status;
    }
    
    m_handle.time.tv_sec = static_cast<time_t>(seconds);
    m_handle.time.tv_nsec = static_cast<long>(nanoseconds);
    
    return Fw::FW_SERIALIZE_OK;
}

RawTimeHandle* RtemsRawTime::getHandle() {
    return &m_handle;
}

} // namespace RawTime
} // namespace RTEMS
} // namespace Os