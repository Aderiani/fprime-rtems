// Os/RTEMS/Queue.cpp - Fixed implementation
#include <Os/Queue.hpp>
#include <Os/RTEMS/Queue.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <cstring>
#include <errno.h>


namespace Os {
namespace RTEMS {
namespace Queue {

QueueInterface* getDelegate(QueueHandleStorage& aligned_placement_new_memory) {
    // Ensure the memory is properly aligned and large enough
    FW_ASSERT(alignof(RTEMSQueue) <= FW_HANDLE_ALIGNMENT, alignof(RTEMSQueue), FW_HANDLE_ALIGNMENT);
    static_assert(sizeof(RTEMSQueue) <= sizeof(QueueHandleStorage), "FW_HANDLE_MAX_SIZE too small");
    
    // Create implementation using placement new
    return new (&aligned_placement_new_memory) RTEMSQueue();
}

RTEMSQueue::RTEMSQueue() {
    // Initialize the queue handle
    m_handle.queue_id = 0;
    m_handle.depth = 0;
    m_handle.msgSize = 0;
    memset(m_handle.name, 0, sizeof(m_handle.name));
}

RTEMSQueue::~RTEMSQueue() {
    if (m_handle.queue_id != 0) {
        rtems_status_code status = rtems_message_queue_delete(m_handle.queue_id);
        if (status != RTEMS_SUCCESSFUL) {
            // Just print the error but don't assert since we're in a destructor
            // We can't do much about deletion failures at this point
            printf("RTEMSQueue: Failed to delete queue: %d\n", status);
        }
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
    
    // Create a unique name from first 4 chars (or pad with spaces)
    char name_chars[5] = "    ";  // Default to spaces
    for (size_t i = 0; i < 4 && i < name.length(); i++) {
        name_chars[i] = name.toChar()[i];
    }
    name_chars[4] = '\0';  // Ensure null termination
    
    rtems_name queue_name = rtems_build_name(
        name_chars[0], name_chars[1], name_chars[2], name_chars[3]
    );
    
    // Create the message queue
    rtems_status_code status = rtems_message_queue_create(
        queue_name,
        depth,
        msgSize,
        RTEMS_PRIORITY,  // Use priority ordering for messages
        &m_handle.queue_id
    );
    
    if (status != RTEMS_SUCCESSFUL) {
        printf("RTEMSQueue: Failed to create queue '%s': %d\n", name.toChar(), status);
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
    
    if (buffer == nullptr) {
        return Status::UNKNOWN_ERROR;
    }
    
    if (size > m_handle.msgSize) {
        return Status::SIZE_MISMATCH;
    }
    
    rtems_option wait_option = (block == BlockingType::BLOCKING) ? RTEMS_WAIT : RTEMS_NO_WAIT;
    
    rtems_status_code status = rtems_message_queue_send(
        m_handle.queue_id,
        buffer,
        size
    );
    
    if (status == RTEMS_SUCCESSFUL) {
        return Status::OP_OK;
    } else if (status == RTEMS_UNSATISFIED) {
        return Status::FULL;
    } else {
        printf("RTEMSQueue: Send error: %d\n", status);
        return Status::SEND_ERROR;
    }
}

QueueInterface::Status RTEMSQueue::receive(U8* destination, FwSizeType capacity, BlockingType block, FwSizeType& actualSize, FwQueuePriorityType& priority) {
    if (m_handle.queue_id == 0) {
        return Status::UNINITIALIZED;
    }
    
    if (destination == nullptr) {
        return Status::UNKNOWN_ERROR;
    }
    
    if (capacity < m_handle.msgSize) {
        return Status::SIZE_MISMATCH;
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
        actualSize = static_cast<FwSizeType>(msg_size);
        priority = 0; // RTEMS doesn't use priority for message queues by default
        return Status::OP_OK;
    } else if (status == RTEMS_UNSATISFIED) {
        return Status::EMPTY;
    } else {
        printf("RTEMSQueue: Receive error: %d\n", status);
        return Status::RECEIVE_ERROR;
    }
}

FwSizeType RTEMSQueue::getMessagesAvailable() const {
    if (m_handle.queue_id == 0) {
        return 0;
    }
    
    uint32_t count = 0;
    rtems_status_code status = rtems_message_queue_get_number_pending(
        m_handle.queue_id,
        &count
    );
    
    return (status == RTEMS_SUCCESSFUL) ? static_cast<FwSizeType>(count) : 0;
}

FwSizeType RTEMSQueue::getMessageHighWaterMark() const {
    // RTEMS doesn't provide a high water mark API
    // Return the configured queue depth as a reasonable default
    return m_handle.depth;
}

QueueHandle* RTEMSQueue::getHandle() {
    return &m_handle;
}

} // namespace Queue
} // namespace RTEMS
} // namespace Os