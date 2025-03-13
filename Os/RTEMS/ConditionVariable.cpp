// ======================================================================
// \title Os/RTEMS/ConditionVariable.cpp
// \brief RTEMS implementation for Os::ConditionVariable
// ======================================================================
#include <Os/RTEMS/ConditionVariable.hpp>
#include <Os/RTEMS/Mutex.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <errno.h>

namespace Os {
namespace RTEMS {
namespace ConditionVariable {

RtemsConditionVariable::RtemsConditionVariable() {
    // Create a RTEMS condition variable
    rtems_status_code status = rtems_condition_variable_create(&m_handle.condition_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
}

RtemsConditionVariable::~RtemsConditionVariable() {
    if (m_handle.condition_id != 0) {
        rtems_status_code status = rtems_condition_variable_delete(m_handle.condition_id);
        FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
    }
}

ConditionVariableInterface::Status RtemsConditionVariable::pend(Mutex& mutex) {
    // Get the RTEMS mutex from the Os::Mutex
    Os::MutexInterface* mutex_if = mutex.getInterface();
    Mutex::RTEMSMutexHandle* rtems_mutex_handle = 
        static_cast<Mutex::RTEMSMutexHandle*>(mutex_if->getHandle());
            
    // Wait on the condition variable
    rtems_status_code status = rtems_condition_variable_wait(
        m_handle.condition_id, 
        rtems_mutex_handle->mutex_id
    );
    
    if (status == RTEMS_SUCCESSFUL) {
        return Status::OP_OK;
    } else {
        return Status::ERROR_OTHER;
    }
}

void RtemsConditionVariable::notify() {
    rtems_status_code status = rtems_condition_variable_signal(m_handle.condition_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
}

void RtemsConditionVariable::notifyAll() {
    rtems_status_code status = rtems_condition_variable_broadcast(m_handle.condition_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
}

ConditionVariableHandle* RtemsConditionVariable::getHandle() {
    return &m_handle;
}

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os