/*
 * Queue.cpp:
 *
 * Implementation of the queue data type.
 *
 *  Created on: July 5th, 2022
 *      Author: lestarch
 *
 */
#include "Queue.hpp"
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>


namespace Types {

Queue::Queue() : m_internal(), m_message_size(0) {
    #ifdef __rtems__
    // For RTEMS, immediately auto-initialize with safe defaults
    static U8 safe_storage[2048]; // Static safe storage
    static bool storage_used = false;
    
    if (!storage_used) {
        storage_used = true;
        FwSizeType default_msg_size = 64;
        FwSizeType default_depth = 2048 / default_msg_size;
        Fw::Logger::log("AUTO-INIT: Creating failsafe Queue");
        // Use internal setup to initialize
        m_message_size = default_msg_size;
        m_internal.setup(safe_storage, 2048);
    }
    #endif
}

void Queue::setup(U8* const storage, const FwSizeType storage_size, const FwSizeType depth, const FwSizeType message_size) {
    #ifdef __rtems__
    // For RTEMS, add extra validation
    if (storage == nullptr) {
        Fw::Logger::log("ERROR: Queue::setup called with null storage");
        return;
    }
    if (message_size <= 0) {
        Fw::Logger::log("ERROR: Queue::setup called with invalid message size: %d", 
                      static_cast<int>(message_size));
        return;
    }
    if (storage_size < depth * message_size) {
        Fw::Logger::log("ERROR: Queue::setup storage too small: needed %u, got %u",
                      static_cast<unsigned>(depth * message_size),
                      static_cast<unsigned>(storage_size));
        return;
    }
    #endif
    
    // Standard logic - ensure enough storage provided
    const FwSizeType total_needed_size = depth * message_size;
    FW_ASSERT(storage_size >= total_needed_size, 
              static_cast<FwAssertArgType>(storage_size), 
              static_cast<FwAssertArgType>(depth), 
              static_cast<FwAssertArgType>(message_size));
    
    m_internal.setup(storage, total_needed_size);
    m_message_size = message_size;
}

// Make all methods defensive:
Fw::SerializeStatus Queue::enqueue(const U8* const message, const FwSizeType size) {
    #ifdef __rtems__
    if (m_message_size <= 0) {
        Fw::Logger::log("ERROR: Queue::enqueue on uninitialized queue");
        return Fw::FW_SERIALIZE_FORMAT_ERROR;
    }
    if (message == nullptr) {
        Fw::Logger::log("ERROR: Queue::enqueue with null message");
        return Fw::FW_SERIALIZE_FORMAT_ERROR;
    }
    if (m_message_size != size) {
        Fw::Logger::log("WARNING: Queue size mismatch: expected=%d, got=%d",
                      static_cast<int>(m_message_size), static_cast<int>(size));
    }
    #else
    // Standard assertions
    FW_ASSERT(m_message_size > 0, static_cast<FwAssertArgType>(m_message_size));
    FW_ASSERT(m_message_size == size, static_cast<FwAssertArgType>(size), 
              static_cast<FwAssertArgType>(m_message_size));
    #endif
    
    return m_internal.serialize(message, m_message_size);
}

Fw::SerializeStatus Queue::dequeue(U8* const message, const FwSizeType size) {
    #ifdef __rtems__
    if (m_message_size <= 0) {
        Fw::Logger::log("ERROR: Queue::dequeue on uninitialized queue");
        return Fw::FW_DESERIALIZE_FORMAT_ERROR;
    }
    if (message == nullptr) {
        Fw::Logger::log("ERROR: Queue::dequeue with null buffer");
        return Fw::FW_DESERIALIZE_FORMAT_ERROR;
    }
    if (m_message_size > size) {
        Fw::Logger::log("ERROR: Queue::dequeue buffer too small: need %d, got %d",
                      static_cast<int>(m_message_size), static_cast<int>(size));
        return Fw::FW_DESERIALIZE_SIZE_MISMATCH;
    }
    #else
    FW_ASSERT(m_message_size > 0);
    FW_ASSERT(m_message_size <= size, static_cast<FwAssertArgType>(size), 
              static_cast<FwAssertArgType>(m_message_size));
    #endif
    
    Fw::SerializeStatus result = m_internal.peek(message, m_message_size, 0);
    if (result != Fw::FW_SERIALIZE_OK) {
        return result;
    }
    return m_internal.rotate(m_message_size);
}

FwSizeType Queue::get_high_water_mark() const {
    #ifdef __rtems__
    if (m_message_size <= 0) {
        return 0;
    }
    #else
    FW_ASSERT(m_message_size > 0, static_cast<FwAssertArgType>(m_message_size));
    #endif
    
    return m_internal.get_high_water_mark() / m_message_size;
}

void Queue::clear_high_water_mark() {
    m_internal.clear_high_water_mark();
}

FwSizeType Queue::getQueueSize() const {
    #ifdef __rtems__
    if (m_message_size <= 0) {
        return 0;
    }
    #else
    FW_ASSERT(m_message_size > 0, static_cast<FwAssertArgType>(m_message_size));
    #endif
    
    return m_internal.get_allocated_size() / m_message_size;
}

} // namespace Types