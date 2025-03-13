// ======================================================================
// \title Os/RTEMS/DefaultCondition.cpp
// \brief sets default Os::Condition to RTEMS implementation via linker
// ======================================================================
#include "Os/Condition.hpp"
#include "Os/RTEMS/ConditionVariable.hpp"
#include "Os/Delegate.hpp"

namespace Os {
ConditionVariableInterface* ConditionVariableInterface::getDelegate(ConditionVariableHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<ConditionVariableInterface, Os::RTEMS::ConditionVariable::RtemsConditionVariable>(aligned_new_memory);
}
}