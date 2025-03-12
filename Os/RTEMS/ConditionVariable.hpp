// ======================================================================
// \title Os/RTEMS/ConditionVariable.hpp
// \brief RTEMS implementation for Os::ConditionVariable
// ======================================================================
#ifndef OS_RTEMS_CONDITIONVARIABLE_HPP
#define OS_RTEMS_CONDITIONVARIABLE_HPP

#include <Os/Condition.hpp>
#include <rtems.h>

namespace Os {
namespace RTEMS {
namespace ConditionVariable {

//! ConditionVariableHandle class definition for RTEMS implementations
struct RtemsConditionVariableHandle : public ConditionVariableHandle {
    rtems_id condition_id;  // RTEMS condition variable identifier
    RtemsConditionVariableHandle() : condition_id(0) {}
};

//! \brief RTEMS implementation of Os::ConditionVariableInterface
class RtemsConditionVariable : public ConditionVariableInterface {
  public:
    //! Constructor
    RtemsConditionVariable();

    //! Destructor
    ~RtemsConditionVariable() override;

    //! Wait on condition variable
    Status wait(Mutex& mutex) override;

    //! Signal one thread waiting on condition variable
    Status signal() override;

    //! Broadcast to all threads waiting on condition variable
    Status broadcast() override;

    //! Get handle to condition variable
    ConditionVariableHandle* getHandle() override;
    
  private:
    RtemsConditionVariableHandle m_handle;
};

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_CONDITIONVARIABLE_HPP