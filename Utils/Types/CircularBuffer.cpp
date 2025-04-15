/*
 * CircularBuffer.cpp:
 *
 * Buffer used to efficiently store data in ring data structure. Uses an externally supplied
 * data store as the backing for this buffer. Thus it is dependent on receiving sole ownership
 * of the supplied buffer.
 *
 * This implementation file contains the function definitions with defensive coding for RTEMS.
 */
#include <FpConfig.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Utils/Types/CircularBuffer.hpp>

namespace Types {

CircularBuffer ::CircularBuffer()
    : m_store(nullptr), m_store_size(0), m_head_idx(0), m_allocated_size(0), m_high_water_mark(0) {
#ifdef __rtems__
    // For RTEMS, immediately auto-initialize with safe defaults
    static U8 safe_storage[2048];  // Static safe storage
    static bool storage_used = false;

    if (!storage_used) {
        storage_used = true;
        Fw::Logger::log("AUTO-INIT: Creating failsafe CircularBuffer");
        // Use internal setup to initialize with safe storage
        m_store = safe_storage;
        m_store_size = sizeof(safe_storage);
        m_head_idx = 0;
        m_allocated_size = 0;
        m_high_water_mark = 0;
    }
#endif
}

CircularBuffer ::CircularBuffer(U8* const buffer, const FwSizeType size)
    : m_store(nullptr), m_store_size(0), m_head_idx(0), m_allocated_size(0), m_high_water_mark(0) {
    setup(buffer, size);
}

void CircularBuffer ::setup(U8* const buffer, const FwSizeType size) {
#ifdef __rtems__
    // For RTEMS, add extra validation
    if (buffer == nullptr) {
        Fw::Logger::log("ERROR: CircularBuffer::setup called with null buffer");
        return;
    }
    if (size <= 0) {
        Fw::Logger::log("ERROR: CircularBuffer::setup called with invalid size: %d", static_cast<int>(size));
        return;
    }
    if (m_store != nullptr) {
        Fw::Logger::log("WARNING: CircularBuffer::setup called when already setup");
        return;
    }
#else
    FW_ASSERT(size > 0);
    FW_ASSERT(buffer != nullptr);
    FW_ASSERT(m_store == nullptr && m_store_size == 0);  // Not already setup
#endif

    // Initialize buffer data
    m_store = buffer;
    m_store_size = size;
    m_head_idx = 0;
    m_allocated_size = 0;
    m_high_water_mark = 0;
}

FwSizeType CircularBuffer ::get_allocated_size() const {
    return m_allocated_size;
}

FwSizeType CircularBuffer ::get_free_size() const {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        return 0;
    }
    if (m_allocated_size > m_store_size) {
        Fw::Logger::log("ERROR: CircularBuffer corrupted: allocated_size > store_size");
        return 0;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
    FW_ASSERT(m_allocated_size <= m_store_size, static_cast<FwAssertArgType>(m_allocated_size));
#endif

    return m_store_size - m_allocated_size;
}

FwSizeType CircularBuffer ::advance_idx(FwSizeType idx, FwSizeType amount) const {
#ifdef __rtems__
    if (m_store_size == 0) {
        return 0;
    }
    if (idx >= m_store_size) {
        Fw::Logger::log("ERROR: CircularBuffer::advance_idx: idx out of bounds: %u >= %u", static_cast<unsigned>(idx),
                        static_cast<unsigned>(m_store_size));
        return 0;
    }
#else
    FW_ASSERT(idx < m_store_size, static_cast<FwAssertArgType>(idx));
#endif

    return (idx + amount) % m_store_size;
}

Fw::SerializeStatus CircularBuffer ::serialize(const U8* const buffer, const FwSizeType size) {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        Fw::Logger::log("ERROR: CircularBuffer::serialize: buffer not initialized");
        return Fw::FW_SERIALIZE_FORMAT_ERROR;
    }
    if (buffer == nullptr) {
        Fw::Logger::log("ERROR: CircularBuffer::serialize: null input buffer");
        return Fw::FW_SERIALIZE_FORMAT_ERROR;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
    FW_ASSERT(buffer != nullptr);
#endif

    // Check there is sufficient space
    if (size > get_free_size()) {
        return Fw::FW_SERIALIZE_NO_ROOM_LEFT;
    }

    // Copy in all the supplied data
    FwSizeType idx = advance_idx(m_head_idx, m_allocated_size);
    for (U32 i = 0; i < size; i++) {
#ifdef __rtems__
        if (idx >= m_store_size) {
            Fw::Logger::log("ERROR: CircularBuffer::serialize: index out of bounds");
            return Fw::FW_SERIALIZE_FORMAT_ERROR;
        }
#else
        FW_ASSERT(idx < m_store_size, static_cast<FwAssertArgType>(idx));
#endif

        m_store[idx] = buffer[i];
        idx = advance_idx(idx);
    }

    m_allocated_size += size;

#ifdef __rtems__
    if (m_allocated_size > this->get_capacity()) {
        Fw::Logger::log("ERROR: CircularBuffer::serialize: allocated > capacity");
        m_allocated_size = this->get_capacity();
    }
#else
    FW_ASSERT(m_allocated_size <= this->get_capacity(), static_cast<FwAssertArgType>(m_allocated_size));
#endif

    m_high_water_mark = (m_high_water_mark > m_allocated_size) ? m_high_water_mark : m_allocated_size;
    return Fw::FW_SERIALIZE_OK;
}

Fw::SerializeStatus CircularBuffer ::peek(char& value, FwSizeType offset) const {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
#endif

    return peek(reinterpret_cast<U8&>(value), offset);
}

Fw::SerializeStatus CircularBuffer ::peek(U8& value, FwSizeType offset) const {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
#endif

    // Check there is sufficient data
    if ((sizeof(U8) + offset) > m_allocated_size) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }

    const FwSizeType idx = advance_idx(m_head_idx, offset);

#ifdef __rtems__
    if (idx >= m_store_size) {
        Fw::Logger::log("ERROR: CircularBuffer::peek: index out of bounds");
        return Fw::FW_DESERIALIZE_FORMAT_ERROR;
    }
#else
    FW_ASSERT(idx < m_store_size, static_cast<FwAssertArgType>(idx));
#endif

    value = m_store[idx];
    return Fw::FW_SERIALIZE_OK;
}

Fw::SerializeStatus CircularBuffer ::peek(U32& value, FwSizeType offset) const {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
#endif

    // Check there is sufficient data
    if ((sizeof(U32) + offset) > m_allocated_size) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }

    value = 0;
    FwSizeType idx = advance_idx(m_head_idx, offset);

    // Deserialize all the bytes from network format
    for (FwSizeType i = 0; i < sizeof(U32); i++) {
#ifdef __rtems__
        if (idx >= m_store_size) {
            Fw::Logger::log("ERROR: CircularBuffer::peek U32: index out of bounds");
            return Fw::FW_DESERIALIZE_FORMAT_ERROR;
        }
#else
        FW_ASSERT(idx < m_store_size, static_cast<FwAssertArgType>(idx));
#endif

        value = (value << 8) | static_cast<U32>(m_store[idx]);
        idx = advance_idx(idx);
    }
    return Fw::FW_SERIALIZE_OK;
}

Fw::SerializeStatus CircularBuffer ::peek(U8* buffer, FwSizeType size, FwSizeType offset) const {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }
    if (buffer == nullptr) {
        Fw::Logger::log("ERROR: CircularBuffer::peek: null output buffer");
        return Fw::FW_DESERIALIZE_FORMAT_ERROR;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
    FW_ASSERT(buffer != nullptr);
#endif

    // Check there is sufficient data
    if ((size + offset) > m_allocated_size) {
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
    }

    FwSizeType idx = advance_idx(m_head_idx, offset);
    // Deserialize all the bytes from network format
    for (FwSizeType i = 0; i < size; i++) {
#ifdef __rtems__
        if (idx >= m_store_size) {
            Fw::Logger::log("ERROR: CircularBuffer::peek buffer: index out of bounds");
            return Fw::FW_DESERIALIZE_FORMAT_ERROR;
        }
#else
        FW_ASSERT(idx < m_store_size, static_cast<FwAssertArgType>(idx));
#endif

        buffer[i] = m_store[idx];
        idx = advance_idx(idx);
    }
    return Fw::FW_SERIALIZE_OK;
}

Fw::SerializeStatus CircularBuffer ::rotate(FwSizeType amount) {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        Fw::Logger::log("ERROR: CircularBuffer::rotate: buffer not initialized");
        return Fw::FW_DESERIALIZE_FORMAT_ERROR;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
#endif

    // Check there is sufficient data
    if (amount > m_allocated_size) {
#ifdef __rtems__
        Fw::Logger::log("ERROR: CircularBuffer::rotate: amount > allocated (%u > %u)", static_cast<unsigned>(amount),
                        static_cast<unsigned>(m_allocated_size));
        return Fw::FW_DESERIALIZE_SIZE_MISMATCH;
#else
        return Fw::FW_DESERIALIZE_BUFFER_EMPTY;
#endif
    }

    m_head_idx = advance_idx(m_head_idx, amount);
    m_allocated_size -= amount;
    return Fw::FW_SERIALIZE_OK;
}

FwSizeType CircularBuffer ::get_capacity() const {
#ifdef __rtems__
    if (m_store == nullptr || m_store_size == 0) {
        return 0;
    }
#else
    FW_ASSERT(m_store != nullptr && m_store_size != 0);  // setup method was called
#endif

    return m_store_size;
}

FwSizeType CircularBuffer ::get_high_water_mark() const {
    return m_high_water_mark;
}

void CircularBuffer ::clear_high_water_mark() {
    m_high_water_mark = 0;
}

}  // End Namespace Types