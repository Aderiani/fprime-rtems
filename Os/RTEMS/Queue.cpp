#include <Os/Queue.hpp>
#include <Os/RTEMS/Queue.hpp>   
#include <rtems.h>
#include <Fw/Types/Assert.hpp>
#include <cstring>

namespace Os {

namespace RTEMS{
namespace Queue {

    Queue::Queue() : m_handle(0), m_depth(0), m_msgSize(0) {
        // Initialize the queue name
        memset(this->m_name, 0, sizeof(this->m_name));
    }

    Queue::~Queue() {
        if (this->m_handle != 0) {
            rtems_message_queue_delete(static_cast<rtems_id>(this->m_handle));
            this->m_handle = 0;
        }
    }

    Queue::Status Queue::create(const Fw::StringBase &name, NATIVE_INT_TYPE depth, NATIVE_INT_TYPE msgSize) {
        if (this->m_handle != 0) {
            return ALREADY_CREATED;
        }
        
        // Store the queue parameters
        this->m_depth = depth;
        this->m_msgSize = msgSize;
        
        // Store the queue name
        strncpy(this->m_name, name.toChar(), sizeof(this->m_name) - 1);
        this->m_name[sizeof(this->m_name) - 1] = 0;
        
        rtems_name queue_name = rtems_build_name(
            (this->m_name[0] ? this->m_name[0] : ' '),
            (this->m_name[1] ? this->m_name[1] : ' '),
            (this->m_name[2] ? this->m_name[2] : ' '),
            (this->m_name[3] ? this->m_name[3] : ' ')
        );
        
        rtems_id queue_id;
        rtems_status_code status = rtems_message_queue_create(
            queue_name,
            depth,
            msgSize,
            RTEMS_DEFAULT_ATTRIBUTES,
            &queue_id
        );
        
        if (status != RTEMS_SUCCESSFUL) {
            switch (status) {
                case RTEMS_INVALID_ADDRESS:
                case RTEMS_INVALID_NAME:
                case RTEMS_INVALID_SIZE:
                    return UNKNOWN_ERROR;
                case RTEMS_TOO_MANY:
                case RTEMS_NO_MEMORY:
                    return UNKNOWN_ERROR;
                default:
                    return UNKNOWN_ERROR;
            }
        }
        
        this->m_handle = queue_id;
        return OP_OK;
    }

    Queue::Status Queue::send(const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority, BlockingType block) {
        if (this->m_handle == 0) {
            return UNINITIALIZED;
        }
        
        if (buffer == nullptr || size <= 0) {
            return UNKNOWN_ERROR;
        }
        
        if (size > this->m_msgSize) {
            return SIZE_MISMATCH;
        }
                
        rtems_status_code status = rtems_message_queue_send(
            static_cast<rtems_id>(this->m_handle),
            buffer,
            size
        );
        
        if (status == RTEMS_SUCCESSFUL) {
            return OP_OK;
        } else if (status == RTEMS_TOO_MANY) {
            return FULL;
        } else {
            return SEND_ERROR;
        }
    }

    Queue::Status Queue::receive(U8* buffer, NATIVE_INT_TYPE& size, NATIVE_INT_TYPE capacity, NATIVE_INT_TYPE& priority, BlockingType block) {
        if (this->m_handle == 0) {
            return UNINITIALIZED;
        }
        
        if (buffer == nullptr || capacity <= 0) {
            return UNKNOWN_ERROR;
        }
        
        rtems_option wait_option = (block == BLOCKING) ? RTEMS_WAIT : RTEMS_NO_WAIT;
        size_t msg_size;
        
        rtems_status_code status = rtems_message_queue_receive(
            static_cast<rtems_id>(this->m_handle),
            buffer,
            &msg_size,
            wait_option,
            RTEMS_NO_TIMEOUT
        );
        
        if (status == RTEMS_SUCCESSFUL) {
            size = msg_size;
            priority = 0; // RTEMS doesn't use priority for queue messages
            return OP_OK;
        } else if (status == RTEMS_UNSATISFIED) {
            return EMPTY;
        } else {
            return RECEIVE_ERROR;
        }
    }

    NATIVE_INT_TYPE Queue::getNumMsgs() const {
        if (this->m_handle == 0) {
            return 0;
        }
        
        uint32_t count;
        rtems_status_code status = rtems_message_queue_get_number_pending(
            static_cast<rtems_id>(this->m_handle),
            &count
        );
        
        return (status == RTEMS_SUCCESSFUL) ? count : 0;
    }

    NATIVE_INT_TYPE Queue::getMaxMsgs() const {
        return this->m_depth;
    }

    NATIVE_INT_TYPE Queue::getMsgSize() const {
        return this->m_msgSize;
    }
}
}
}
