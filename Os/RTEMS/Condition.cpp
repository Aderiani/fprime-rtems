// ======================================================================
// \title Os/RTEMS/ConditionVariable.cpp
// \brief RTEMS implementation for Os::ConditionVariable
// ======================================================================
#include "Os/Condition.hpp"
#include "Os/RTEMS/Mutex.hpp"
#include "Os/Delegate.hpp"
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <errno.h>

namespace Os {
namespace RTEMS {
namespace ConditionVariable {

struct RtemsConditionVariableHandle : public ConditionVariableHandle {
    rtems_id condition_id;  // RTEMS condition variable identifier
    RtemsConditionVariableHandle() : condition_id(0) {}
};

class RtemsConditionVariable : public ConditionVariableInterface {
  public:
    //! Constructor
    RtemsConditionVariable() {
        // Create a RTEMS condition variable
        rtems_status_code status = rtems_condition_variable_create(&m_handle.condition_id);
        FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
    }

    //! Destructor
    ~RtemsConditionVariable() override {
        if (m_handle.condition_id != 0) {
            rtems_status_code status = rtems_condition_variable_delete(m_handle.condition_id);
            FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
        }
    }

    //! Wait on condition variable
    Status wait(Mutex& mutex) override {
        // Get the RTEMS mutex from the Os::Mutex
        Os::MutexInterface* mutex_if = mutex.getInterface();
        RTEMSMutex::RTEMSMutexHandle* rtems_mutex_handle = 
            static_cast<RTEMSMutex::RTEMSMutexHandle*>(mutex_if->getHandle());
            
        // Wait on the condition variable
        rtems_status_code status = rtems_condition_variable_wait(
            m_handle.condition_id, 
            rtems_mutex_handle->mutex_id
        );
        
        if (status == RTEMS_SUCCESSFUL) {
            return Status::OK;
        } else {
            return Status::ERROR;
        }
    }

    //! Signal one thread waiting on condition variable
    Status signal() override {
        rtems_status_code status = rtems_condition_variable_signal(m_handle.condition_id);
        
        if (status == RTEMS_SUCCESSFUL) {
            return Status::OK;
        } else {
            return Status::ERROR;
        }
    }

    //! Broadcast to all threads waiting on condition variable
    Status broadcast() override {
        rtems_status_code status = rtems_condition_variable_broadcast(m_handle.condition_id);
        
        if (status == RTEMS_SUCCESSFUL) {
            return Status::OK;
        } else {
            return Status::ERROR;
        }
    }

    //! Get handle to condition variable
    ConditionVariableHandle* getHandle() override {
        return &m_handle;
    }
    
  private:
    RtemsConditionVariableHandle m_handle;
};

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os

namespace Os {
ConditionVariableInterface* ConditionVariableInterface::getDelegate(ConditionVariableHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<ConditionVariableInterface, Os::RTEMS::ConditionVariable::RtemsConditionVariable>(aligned_new_memory);
}
}