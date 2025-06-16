
#include <errno.h>
#include <malloc.h>  // for memalign
#include <rtems.h>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Delegate.hpp>
#include <Os/Queue.hpp>
#include <Os/RTEMS/Queue.hpp>
#include <cstdint>  // for uintptr_t
#include <cstdlib>  // for free
#include <cstring>

namespace Os {
namespace RTEMS {
namespace Queue {

QueueInterface* getDelegate(QueueHandleStorage& aligned_placement_new_memory) {
    static_assert(sizeof(RTEMSQueue) <= sizeof(QueueHandleStorage), "FW_HANDLE_MAX_SIZE too small");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(RTEMSQueue)) == 0, "Bad handle alignment");

    // Create implementation using placement new
    return new (&aligned_placement_new_memory) RTEMSQueue();
}

RTEMSQueue::RTEMSQueue() {
    // Initialize the queue handle
    m_handle.queue_id = 0;
    m_handle.depth = 0;
    m_handle.msgSize = 0;
    memset(m_handle.name, 0, sizeof(m_handle.name));

    // Fw::Logger::log("RTEMSQueue: Initialized at %p\n", this);
}

RTEMSQueue::~RTEMSQueue() {
    Fw::Logger::log("RTEMSQueue: Destroying queue at %p, id=%d\n", this, m_handle.queue_id);

    if (m_handle.queue_id != 0) {
        rtems_status_code status = rtems_message_queue_delete(m_handle.queue_id);
        if (status != RTEMS_SUCCESSFUL) {
            Fw::Logger::log("RTEMSQueue: Failed to delete queue: %d\n", status);
        }
        m_handle.queue_id = 0;
    }
}

QueueInterface::Status RTEMSQueue::create(const Fw::StringBase& name, FwSizeType depth, FwSizeType msgSize) {
    if (m_handle.queue_id != 0) {
        Fw::Logger::log("RTEMSQueue: Queue already created: %s\n", name.toChar());
        return QueueInterface::Status::ALREADY_CREATED;
    }

    // Store the ACTUAL depth and msgSize
    m_handle.depth = depth;
    m_handle.msgSize = msgSize;  

    // Store the queue name
    strncpy(m_handle.name, name.toChar(), sizeof(m_handle.name) - 1);
    m_handle.name[sizeof(m_handle.name) - 1] = 0;

    // Create unique name from first 4 chars
    char name_chars[5] = "QQQQ";
    for (size_t i = 0; i < 4 && i < name.length(); i++) {
        name_chars[i] = name.toChar()[i];
    }

    rtems_name queue_name = rtems_build_name(name_chars[0], name_chars[1], name_chars[2], name_chars[3]);

    // Fw::Logger::log("RTEMSQueue: Creating queue '%s' with depth %d, msgSize %d\n", name.toChar(), (int)depth,
    //                 (int)m_handle.msgSize);

    // PASS THE DEPTH PARAMETER TO THE RTEMS FUNCTION
    rtems_status_code status = rtems_message_queue_create(queue_name,
                                                          depth,  // Pass the actual depth parameter
                                                          m_handle.msgSize, RTEMS_FIFO, &m_handle.queue_id);

    if (status != RTEMS_SUCCESSFUL) {
        Fw::Logger::log("RTEMSQueue: Failed to create queue '%s': error %d\n", name.toChar(), status);
        return QueueInterface::Status::UNKNOWN_ERROR;
    }

    // Fw::Logger::log("RTEMSQueue: Created queue '%s' with depth %d, msgSize %d, id=%d\n", name.toChar(), (int)depth,
    //                 (int)m_handle.msgSize, m_handle.queue_id);

    return QueueInterface::Status::OP_OK;
}

QueueInterface::Status RTEMSQueue::send(const U8* buffer,
                                        FwSizeType size,
                                        FwQueuePriorityType priority,
                                        BlockingType block) {
    if (m_handle.queue_id == 0) {
        Fw::Logger::log("RTEMSQueue: Send error - queue not initialized\n");
        return QueueInterface::Status::UNINITIALIZED;
    }

    if (buffer == nullptr) {
        Fw::Logger::log("RTEMSQueue: Send error - null buffer\n");
        return QueueInterface::Status::UNKNOWN_ERROR;
    }

    if (size > static_cast<FwSizeType>(m_handle.msgSize)) {
        Fw::Logger::log("RTEMSQueue: Send error - size mismatch: %d > %d\n", size, m_handle.msgSize);
        return QueueInterface::Status::SIZE_MISMATCH;
    }

    // Prepare aligned message buffer
    void* aligned_buffer = memalign(8, m_handle.msgSize);
    if (aligned_buffer == nullptr) {
        Fw::Logger::log("RTEMSQueue: Send error - failed to allocate aligned buffer\n");
        return QueueInterface::Status::UNKNOWN_ERROR;
    }

    // Clear the entire buffer first to avoid uninitialized memory
    memset(aligned_buffer, 0, m_handle.msgSize);

    // Copy actual data
    memcpy(aligned_buffer, buffer, size);

    // Send with proper options
    rtems_option send_option = (block == BlockingType::BLOCKING) ? RTEMS_WAIT : RTEMS_NO_WAIT;

    rtems_status_code status = rtems_message_queue_send(m_handle.queue_id, aligned_buffer,
                                                        size  // Send actual data size, not padded size
    );

    // Free the aligned buffer as it's been copied by RTEMS
    free(aligned_buffer);

    if (status == RTEMS_SUCCESSFUL) {
        return QueueInterface::Status::OP_OK;
    } else if (status == RTEMS_UNSATISFIED && block == BlockingType::NONBLOCKING) {
        Fw::Logger::log("RTEMSQueue: Queue full (id=%d)\n", m_handle.queue_id);
        return QueueInterface::Status::FULL;
    } else {
        Fw::Logger::log("RTEMSQueue: Send error: %d (0x%x)\n", status, status);

        // Check if queue was deleted or is invalid
        if (status == RTEMS_INVALID_ID || status == RTEMS_OBJECT_WAS_DELETED) {
            m_handle.queue_id = 0;  // Mark as invalid
            return QueueInterface::Status::UNINITIALIZED;
        }

        return QueueInterface::Status::SEND_ERROR;
    }
}


QueueInterface::Status RTEMSQueue::receive(U8* destination,
                                           FwSizeType capacity,
                                           BlockingType block,
                                           FwSizeType& actualSize,
                                           FwQueuePriorityType& priority) {
    
    // printf("[RTEMS-QUEUE] Receive for '%s' with capacity=%d, msgSize=%d\n", 
    //        m_handle.name, (int)capacity, (int)m_handle.msgSize);
    
    if (m_handle.queue_id == 0) {
        // printf("[RTEMS-QUEUE] Receive error - queue not initialized\n");
        return QueueInterface::Status::UNINITIALIZED;
    }

    if (destination == nullptr) {
        // printf("[RTEMS-QUEUE] Receive error - null destination\n");
        return QueueInterface::Status::UNKNOWN_ERROR;
    }

    if (capacity == 0) {
        // printf("[RTEMS-QUEUE] Critical error - zero capacity buffer\n");
        actualSize = 0;
        priority = 0;
        return QueueInterface::Status::SIZE_MISMATCH;
    }

    if (capacity < m_handle.msgSize) {
        // printf("[RTEMS-QUEUE] Receive error - capacity too small: %d < %d\n", 
            //    (int)capacity, (int)m_handle.msgSize);
        return QueueInterface::Status::SIZE_MISMATCH;
    }

    // Check if queue is still valid before receiving
    uint32_t pending = 0;
    rtems_status_code check_status = rtems_message_queue_get_number_pending(m_handle.queue_id, &pending);

    if (check_status != RTEMS_SUCCESSFUL) {
        // printf("[RTEMS-QUEUE] Receive error - queue check failed: %d\n", check_status);
        if (check_status == RTEMS_INVALID_ID || check_status == RTEMS_OBJECT_WAS_DELETED) {
            m_handle.queue_id = 0;
            return QueueInterface::Status::UNINITIALIZED;
        }
        return QueueInterface::Status::RECEIVE_ERROR;
    }

    // Special case for non-blocking: if empty, return immediately
    if (block == BlockingType::NONBLOCKING && pending == 0) {
        // printf("[RTEMS-QUEUE] Queue empty for non-blocking receive\n");
        return QueueInterface::Status::EMPTY;
    }

    // printf("[RTEMS-QUEUE] Queue has %u pending messages\n", pending);

    rtems_option wait_option = (block == BlockingType::BLOCKING) ? RTEMS_WAIT : RTEMS_NO_WAIT;
    size_t msg_size = 0;

    // FIXED: Receive directly into destination buffer
    rtems_status_code status = rtems_message_queue_receive(m_handle.queue_id,
                                                           destination,     // Receive directly here
                                                           &msg_size, 
                                                           wait_option, 
                                                           RTEMS_NO_TIMEOUT);

    if (status == RTEMS_SUCCESSFUL) {
        // printf("[RTEMS-QUEUE] Successfully received %zu bytes\n", msg_size);
        
        // // Debug: Print first few bytes of received message
        // printf("[RTEMS-QUEUE] Message data: ");
        // for (size_t i = 0; i < (msg_size > 8 ? 8 : msg_size); i++) {
        //     printf("%02X ", destination[i]);
        // }
        // printf("\n");
        
        actualSize = static_cast<FwSizeType>(msg_size);
        priority = 0;  // RTEMS doesn't use priority for message queues by default
        return QueueInterface::Status::OP_OK;
    } else {
        if (status == RTEMS_UNSATISFIED) {
            printf("[RTEMS-QUEUE] Queue empty (id=%d)\n", m_handle.queue_id);
            return QueueInterface::Status::EMPTY;
        } else {
            printf("[RTEMS-QUEUE] Receive error: %d (0x%x)\n", status, status);

            // Check if queue was deleted or is invalid
            if (status == RTEMS_INVALID_ID || status == RTEMS_OBJECT_WAS_DELETED) {
                m_handle.queue_id = 0;
                return QueueInterface::Status::UNINITIALIZED;
            }

            return QueueInterface::Status::RECEIVE_ERROR;
        }
    }
}


FwSizeType RTEMSQueue::getMessagesAvailable() const {
    if (m_handle.queue_id == 0) {
        return 0;
    }

    uint32_t count = 0;
    rtems_status_code status = rtems_message_queue_get_number_pending(m_handle.queue_id, &count);

    if (status != RTEMS_SUCCESSFUL) {
        Fw::Logger::log("RTEMSQueue: Failed to get pending count: %d\n", status);
        return 0;
    }

    return static_cast<FwSizeType>(count);
}

FwSizeType RTEMSQueue::getMessageHighWaterMark() const {
    // RTEMS doesn't provide a high water mark API
    // Return the configured queue depth as a reasonable default
    return m_handle.depth;
}

QueueHandle* RTEMSQueue::getHandle() {
    return &m_handle;
}

}  // namespace Queue
}  // namespace RTEMS
}  // namespace Os
