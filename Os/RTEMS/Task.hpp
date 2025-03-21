// ======================================================================
// \title Os/RTEMS/Task.hpp
// \brief RTEMS implementation for Os::Task, header and test definitions
// ======================================================================
#include <Os/Task.hpp>
#include <rtems.h>

#ifndef OS_RTEMS_TASK_HPP
#define OS_RTEMS_TASK_HPP

namespace Os {
namespace RTEMS {
namespace Task {

//! TaskHandle class definition for RTEMS implementations.
struct RTEMSTaskHandle : public TaskHandle {
    rtems_id task_id;  // RTEMS task identifier
};

//! \brief RTEMS implementation of Os::TaskInterface
class RTEMSTask : public TaskInterface {
  public:
    //! \brief constructor
    RTEMSTask() = default;

    //! \brief copy constructor is forbidden
    RTEMSTask(const RTEMSTask& other) = delete;

    //! \brief assignment operator is forbidden
    TaskInterface& operator=(const TaskInterface& other) = delete; 

    //! \brief destructor
    ~RTEMSTask() override = default;

    // ------------------------------------
    // TaskInterface implementation
    // ------------------------------------
    void onStart() override;
    Status start(const Arguments& arguments) override;
    Status join() override;
    void suspend(SuspensionType suspensionType) override;
    void resume() override;
    Status _delay(Fw::TimeInterval interval) override;
    TaskHandle* getHandle() override;

  private:
    RTEMSTaskHandle m_handle;  //!< RTEMS task handle
};

}  // namespace Task
}  // namespace RTEMS
}  // namespace Os

#endif  // OS_RTEMS_TASK_HPP