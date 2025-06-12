#ifndef _Os_RTEMS_Queue_hpp_
#define _Os_RTEMS_Queue_hpp_

#include <rtems.h>
#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/StringBase.hpp>
#include <Os/Queue.hpp>
#include <cstdio>  // For printf

namespace Os {
namespace RTEMS {
namespace Queue {

/**
 * RTEMS-specific queue handle structure
 * This maintains all the information needed for an RTEMS message queue
 */
struct RTEMSQueueHandle : public QueueHandle {
    rtems_id queue_id;        ///< RTEMS queue ID
    char name[80];            ///< Queue name
    NATIVE_INT_TYPE depth;    ///< Queue depth
    NATIVE_INT_TYPE msgSize;  ///< Maximum message size
};

/**
 * RTEMS-specific queue implementation
 */
class RTEMSQueue : public QueueInterface {
  public:
    /**
     * Constructor
     */
    RTEMSQueue();

    /**
     * Destructor
     * Cleans up resources if queue was created
     */
    ~RTEMSQueue() override;

    /**
     * Copy constructor - deleted
     */
    RTEMSQueue(const RTEMSQueue& other) = delete;

    /**
     * Assignment operator - deleted (this fixes the first error)
     */
    RTEMSQueue& operator=(const QueueInterface& other) override = delete;

    /**
     * Create a queue
     * @param name Queue name
     * @param depth Maximum number of messages the queue can hold
     * @param msgSize Maximum size of each message
     * @return Status::OP_OK on success, appropriate error otherwise
     */
    Status create(const Fw::StringBase& name, FwSizeType depth, FwSizeType msgSize) override;

    /**
     * Send a message to the queue
     * @param buffer Message data
     * @param size Message size
     * @param priority Message priority
     * @param block Whether to block if queue is full
     * @return Status::OP_OK on success, appropriate error otherwise
     */
    Status send(const U8* buffer, FwSizeType size, FwQueuePriorityType priority, BlockingType block) override;

    /**
     * Receive a message from the queue
     * @param destination Buffer to store the message
     * @param capacity Buffer capacity
     * @param block Whether to block if queue is empty
     * @param actualSize [out] Actual size of received message
     * @param priority [out] Priority of received message
     * @return Status::OP_OK on success, appropriate error otherwise
     */
    Status receive(U8* destination,
                   FwSizeType capacity,
                   BlockingType block,
                   FwSizeType& actualSize,
                   FwQueuePriorityType& priority) override;

    /**
     * Get number of messages currently in the queue
     * @return Number of messages
     */
    FwSizeType getMessagesAvailable() const override;

    /**
     * Get high water mark (maximum number of messages in queue at any time)
     * @return High water mark
     */
    FwSizeType getMessageHighWaterMark() const override;

    /**
     * Get handle
     * @return Handle pointer
     */
    QueueHandle* getHandle() override;

    /**
     * Get minimum buffer size (fixed the sign conversion error)
     */
    FwSizeType getMinBufferSize() const { 
        return static_cast<FwSizeType>(m_handle.msgSize); 
    }

  private:
    RTEMSQueueHandle m_handle;  ///< RTEMS queue handle
};

}  // namespace Queue
}  // namespace RTEMS
}  // namespace Os

#endif  // _Os_RTEMS_Queue_hpp_