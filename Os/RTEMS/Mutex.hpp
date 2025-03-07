// ======================================================================
// \title Os/RTEMS/Mutex.hpp
// \brief RTEMS implementation for Os::Mutex, header and test definitions
// ======================================================================
#include <Os/Mutex.hpp>
#include <rtems.h>

#ifndef OS_RTEMS_MUTEX_HPP
#define OS_RTEMS_MUTEX_HPP

namespace Os {
namespace RTEMS {
namespace Mutex {

//! MutexHandle class definition for RTEMS implementations.
struct RTEMSMutexHandle : public MutexHandle {
    rtems_id mutex_id;  // RTEMS mutex identifier
};

//! \brief RTEMS implementation of Os::MutexInterface
class RTEMSMutex : public MutexInterface {
  public:
    //! \brief constructor
    RTEMSMutex();

    //! \brief copy constructor is forbidden
    RTEMSMutex(const RTEMSMutex& other) = delete;

    //! \brief assignment operator is forbidden
    MutexInterface& operator=(const MutexInterface& other) override = delete;

    //! \brief destructor
    ~RTEMSMutex() override;

    // ------------------------------------
    // MutexInterface implementation
    // ------------------------------------
    Status take() override;
    Status release() override;
    MutexHandle* getHandle() override;

  private:
    RTEMSMutexHandle m_handle;  //!< RTEMS mutex handle
};

}  // namespace Mutex
}  // namespace RTEMS
}  // namespace Os

#endif  // OS_RTEMS_MUTEX_HPP