#ifndef _Os_RTEMS_Queue_hpp_
#define _Os_RTEMS_Queue_hpp_

#include <Os/Queue.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/StringBase.hpp>
#include <rtems.h>

namespace Os {
namespace RTEMS {
namespace Queue {

struct RTEMSQueueHandle : public QueueHandle {
    rtems_id queue_id;
    char name[80];
    NATIVE_INT_TYPE depth;
    NATIVE_INT_TYPE msgSize;
};

class RTEMSQueue : public QueueInterface {
  public:
    RTEMSQueue();
    ~RTEMSQueue() override;

    Status create(const Fw::StringBase &name, FwSizeType depth, FwSizeType msgSize) override;
    Status send(const U8* buffer, FwSizeType size, FwQueuePriorityType priority, BlockingType block) override;
    Status receive(U8* destination, FwSizeType capacity, BlockingType block, FwSizeType& actualSize, FwQueuePriorityType& priority) override;
    FwSizeType getMessagesAvailable() const override;
    FwSizeType getMessageHighWaterMark() const override;
    QueueHandle* getHandle() override;

  private:
    RTEMSQueueHandle m_handle;
};

} // namespace Queue
} // namespace RTEMS
} // namespace Os

#endif // _Os_RTEMS_Queue_hpp_