// ======================================================================
// \title Os/RTEMS/RawTime.cpp
// \brief RTEMS implementation for Os::RawTime
// ======================================================================
#include <Os/RawTime.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <sys/time.h>
#include <time.h>

namespace Os {
namespace RTEMS {
namespace RawTime {

struct RtemsRawTimeHandle : public RawTimeHandle {
    struct timespec time;
};

class RtemsRawTime : public RawTimeInterface {
  public:
    //! Constructor
    RtemsRawTime() {
        m_handle.time.tv_sec = 0;
        m_handle.time.tv_nsec = 0;
    }

    //! Destructor
    ~RtemsRawTime() override = default;

    //! Get current time
    Status now() override {
        if (clock_gettime(CLOCK_MONOTONIC, &m_handle.time) != 0) {
            return Status::OTHER_ERROR;
        }
        return Status::OP_OK;
    }

    //! Calculate time interval between this and another raw time
    Status getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const override {
        // Cast to our implementation
        const RtemsRawTime* otherTime = static_cast<const RtemsRawTime*>(
            dynamic_cast<const RawTimeInterface*>(&other));
            
        if (otherTime == nullptr) {
            return Status::INVALID_PARAMS;
        }
        
        // Calculate difference in seconds and nanoseconds
        time_t sec_diff = m_handle.time.tv_sec - otherTime->m_handle.time.tv_sec;
        long nsec_diff = m_handle.time.tv_nsec - otherTime->m_handle.time.tv_nsec;
        
        // Adjust for negative nanoseconds
        if (nsec_diff < 0) {
            sec_diff--;
            nsec_diff += 1000000000L;
        }
        
        // Check for overflow - shouldn't happen with 64-bit types
        if (sec_diff > static_cast<time_t>(std::numeric_limits<U32>::max())) {
            return Status::OP_OVERFLOW;
        }
        
        interval.set(static_cast<U32>(sec_diff), static_cast<U32>(nsec_diff));
        return Status::OP_OK;
    }

    //! Serialize the raw time
    Fw::SerializeStatus serialize(Fw::SerializeBufferBase& buffer) const override {
        // Serialize seconds and nanoseconds
        Fw::SerializeStatus status;
        
        status = buffer.serialize(static_cast<U32>(m_handle.time.tv_sec));
        if (status != Fw::FW_SERIALIZE_OK) {
            return status;
        }
        
        status = buffer.serialize(static_cast<U32>(m_handle.time.tv_nsec));
        return status;
    }

    //! Deserialize the raw time
    Fw::SerializeStatus deserialize(Fw::SerializeBufferBase& buffer) override {
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

    //! Get raw time handle
    RawTimeHandle* getHandle() override {
        return &m_handle;
    }

  private:
    RtemsRawTimeHandle m_handle;
};

} // namespace RawTime
} // namespace RTEMS
} // namespace Os

namespace Os {
RawTimeInterface* RawTimeInterface::getDelegate(RawTimeHandleStorage& aligned_new_memory) {
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::RawTime::RtemsRawTime) <= sizeof(RawTimeHandleStorage),
                  "RTEMS RawTime implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::RawTime::RtemsRawTime)) == 0,
                  "Bad alignment for RTEMS RawTime implementation");
    return new (aligned_new_memory) Os::RTEMS::RawTime::RtemsRawTime;
}
}