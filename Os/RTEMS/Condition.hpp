// ======================================================================
// \title Os/RTEMS/ConditionVariable.hpp
// \brief RTEMS-specific declarations for Os::ConditionVariable
// ======================================================================

#ifndef OS_RTEMS_CONDITION_VARIABLE_HPP_
#define OS_RTEMS_CONDITION_VARIABLE_HPP_

#include "Os/Condition.hpp"
#include "Os/RTEMS/Mutex.hpp"
#include <pthread.h>  // For POSIX condition variables

namespace Os {
namespace RTEMS {
namespace ConditionVariable {

//! \brief RTEMS-specific condition variable handle
struct RtemsConditionVariableHandle : public ConditionVariableHandle {
    pthread_cond_t condition;  //!< POSIX condition variable
};

//! \brief RTEMS implementation of ConditionVariableInterface
class RtemsConditionVariable : public ConditionVariableInterface {
  public:
    //! Constructor
    RtemsConditionVariable();

    //! Destructor
    ~RtemsConditionVariable() override;

    //! Wait on condition variable
    Status pend(Os::Mutex& mutex) override;

    //! Signal one thread
    void notify() override;

    //! Broadcast to all threads
    void notifyAll() override;

    //! Get handle to condition variable
    ConditionVariableHandle* getHandle() override;

  private:
    RtemsConditionVariableHandle m_handle;  //!< Internal handle storage
};

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_CONDITION_VARIABLE_HPP_