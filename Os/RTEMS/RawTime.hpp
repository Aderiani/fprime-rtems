// ======================================================================
// \title Os/RTEMS/RawTime.hpp
// \brief RTEMS implementation for Os::RawTime
// ======================================================================
#ifndef OS_RTEMS_RAWTIME_HPP
#define OS_RTEMS_RAWTIME_HPP

#include <Os/RawTime.hpp>
#include <rtems.h>
#include <sys/time.h>
#include <time.h>

namespace Os {
namespace RTEMS {
namespace RawTime {

//! RawTimeHandle class definition for RTEMS implementations
struct RtemsRawTimeHandle : public RawTimeHandle {
    struct timespec time;
};

//! \brief RTEMS implementation of RawTimeInterface
class RtemsRawTime : public RawTimeInterface {
  public:
    //! Constructor
    RtemsRawTime();

    //! Destructor
    ~RtemsRawTime() override = default;

    //! Get current time
    Status now() override;

    //! Calculate time interval between this and another raw time
    Status getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const override;

    //! Serialize the raw time
    Fw::SerializeStatus serialize(Fw::SerializeBufferBase& buffer) const override;

    //! Deserialize the raw time
    Fw::SerializeStatus deserialize(Fw::SerializeBufferBase& buffer) override;

    //! Get raw time handle
    RawTimeHandle* getHandle() override;

  private:
    RtemsRawTimeHandle m_handle;
};

} // namespace RawTime
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_RAWTIME_HPP