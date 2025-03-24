#include <Os/Queue.hpp>
#include <Os/RTEMS/Queue.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <cstring>

namespace Os {
namespace RTEMS {
namespace Queue {

RTEMSQueue::RTEMSQueue() {
    // Initialize the queue handle
    m_handle.queue_id = 0;
    m_handle.depth = 0;
    m_handle.msgSize = 0;
    memset(m_handle.name, 0, sizeof(m_handle.name));
}

RTEMSQueue::~RTEMSQueue() {
    if (m_handle.queue_id != 0) {
        rtems_message_queue_delete(m_handle.queue_id);
        m_handle.queue_id = 0;
    }
}

QueueInterface::Status RTEMSQueue::create(const Fw::StringBase &name, FwSizeType depth, FwSizeType msgSize) {
    if (m_handle.queue_id != 0) {
        return Status::ALREADY_CREATED;
    }
    
    // Store the queue parameters
    m_handle.depth = depth;
    m_handle.msgSize = msgSize;
    
    // Store the queue name
    strncpy(m_handle.name, name.toChar(), sizeof(m_handle.name) - 1);
    m_handle.name[sizeof(m_handle.name) - 1] = 0;
    
    rtems_name queue_name = rtems_build_name(
        (m_handle.name[0] ? m_handle.name[0] : ' '),
        (m_handle.name[1] ? m_handle.name[1] : ' '),
        (m_handle.name[2] ? m_handle.name[2] : ' '),
        (m_handle.name[3] ? m_handle.name[3] : ' ')
    );
    
    rtems_status_code status = rtems_message_queue_create(
        queue_name,
        depth,
        msgSize,
        RTEMS_DEFAULT_ATTRIBUTES,
        &m_handle.queue_id
    );
    
    if (status != RTEMS_SUCCESSFUL) {
        switch (status) {
            case RTEMS_INVALID_ADDRESS:
            case RTEMS_INVALID_NAME:
            case RTEMS_INVALID_SIZE:
                return Status::UNKNOWN_ERROR;
            case RTEMS_TOO_MANY:
            case RTEMS_NO_MEMORY:
                return Status::UNKNOWN_ERROR;
            default:
                return Status::UNKNOWN_ERROR;
        }
    }
    
    return Status::OP_OK;
}

QueueInterface::Status RTEMSQueue::send(const U8* buffer, FwSizeType size, FwQueuePriorityType priority, BlockingType block) {
    if (m_handle.queue_id == 0) {
        return Status::UNINITIALIZED;
    }
    
    if (buffer == nullptr || size <= 0) {
        return Status::UNKNOWN_ERROR;
    }
    
    if (static_cast<FwSizeType>(size) > static_cast<FwSizeType>(m_handle.msgSize)) {
        return Status::SIZE_MISMATCH;
    }
        
    rtems_status_code status = rtems_message_queue_send(
        m_handle.queue_id,
        buffer,
        size
    );
    
    if (status == RTEMS_SUCCESSFUL) {
        return Status::OP_OK;
    } else if (status == RTEMS_TOO_MANY) {
        return Status::FULL;
    } else {
        return Status::SEND_ERROR;
    }
}

QueueInterface::Status RTEMSQueue::receive(U8* destination, FwSizeType capacity, BlockingType block, FwSizeType& actualSize, FwQueuePriorityType& priority) {
    if (m_handle.queue_id == 0) {
        return Status::UNINITIALIZED;
    }
    
    if (destination == nullptr || capacity <= 0) {
        return Status::UNKNOWN_ERROR;
    }
    
    rtems_option wait_option = (block == BlockingType::BLOCKING) ? RTEMS_WAIT : RTEMS_NO_WAIT;
    size_t msg_size;
    
    rtems_status_code status = rtems_message_queue_receive(
        m_handle.queue_id,
        destination,
        &msg_size,
        wait_option,
        RTEMS_NO_TIMEOUT
    );
    
    if (status == RTEMS_SUCCESSFUL) {
        actualSize = msg_size;
        priority = 0; // RTEMS doesn't use priority for queue messages
        return Status::OP_OK;
    } else if (status == RTEMS_UNSATISFIED) {
        return Status::EMPTY;
    } else {
        return Status::RECEIVE_ERROR;
    }
}

FwSizeType RTEMSQueue::getMessagesAvailable() const {
    if (m_handle.queue_id == 0) {
        return 0;
    }
    
    uint32_t count;
    rtems_status_code status = rtems_message_queue_get_number_pending(
        m_handle.queue_id,
        &count
    );
    
    return (status == RTEMS_SUCCESSFUL) ? count : 0;
}

FwSizeType RTEMSQueue::getMessageHighWaterMark() const {
    // RTEMS doesn't provide a high water mark API, so we return the queue depth
    return m_handle.depth;
}

QueueHandle* RTEMSQueue::getHandle() {
    return &m_handle;
}

} // namespace Queue
} // namespace RTEMS
} // namespace Os