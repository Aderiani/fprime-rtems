// ======================================================================
// \title  ComQueue.cpp
// \author vbai
// \brief  cpp file for ComQueue component implementation class
// ======================================================================

#include <Fw/Types/Assert.hpp>
#include <Svc/ComQueue/ComQueue.hpp>
#include "Fw/Types/BasicTypes.hpp"
#define DEBUG_COMQUEUE 1  // Set to 0 to disable debug prints

namespace Svc {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

ComQueue ::QueueConfigurationTable ::QueueConfigurationTable() {
    for (NATIVE_UINT_TYPE i = 0; i < FW_NUM_ARRAY_ELEMENTS(this->entries); i++) {
        this->entries[i].priority = 0;
        this->entries[i].depth = 0;
    }
}

ComQueue ::ComQueue(const char* const compName)
    : ComQueueComponentBase(compName),
      m_state(WAITING),
      m_allocationId(static_cast<FwEnumStoreType>(-1)),
      m_allocator(nullptr),
      m_allocation(nullptr) {
    // Initialize throttles to "off"
    for (FwIndexType i = 0; i < TOTAL_PORT_COUNT; i++) {
        this->m_throttle[i] = false;
    }
}

ComQueue ::~ComQueue() {}

void ComQueue ::cleanup() {
    // Deallocate memory ignoring error conditions
    if ((this->m_allocator != nullptr) && (this->m_allocation != nullptr)) {
        this->m_allocator->deallocate(this->m_allocationId, this->m_allocation);
    }
}

void ComQueue::configure(QueueConfigurationTable queueConfig,
                         FwEnumStoreType allocationId,
                         Fw::MemAllocator& allocator) {
    FwIndexType currentPriorityIndex = 0;
    FwSizeType totalAllocation = 0;

    // Store/initialize allocator members
    this->m_allocator = &allocator;
    this->m_allocationId = allocationId;
    this->m_allocation = nullptr;

    // Initializes the sorted queue metadata list in priority (sorted) order. This is accomplished by walking the
    // priority values in priority order from 0 to TOTAL_PORT_COUNT. At each priory value, the supplied queue
    // configuration table is walked and any entry matching the current priority values is used to add queue metadata to
    // the prioritized list. This results in priority-sorted queue metadata objects that index back into the unsorted
    // queue data structures.
    //
    // The total allocation size is tracked for passing to the allocation call and is a summation of
    // (depth * message size)  for each prioritized metadata object of (depth * message size)
    for (FwIndexType currentPriority = 0; currentPriority < TOTAL_PORT_COUNT; currentPriority++) {
        // Walk each queue configuration entry and add them into the prioritized metadata list when matching the current
        // priority value
        for (FwIndexType entryIndex = 0;
             entryIndex < static_cast<FwIndexType>(FW_NUM_ARRAY_ELEMENTS(queueConfig.entries)); entryIndex++) {
            // Check for valid configuration entry
            FW_ASSERT(queueConfig.entries[entryIndex].priority < TOTAL_PORT_COUNT,
                      static_cast<FwAssertArgType>(queueConfig.entries[entryIndex].priority),
                      static_cast<FwAssertArgType>(TOTAL_PORT_COUNT), static_cast<FwAssertArgType>(entryIndex));

            if (currentPriority == queueConfig.entries[entryIndex].priority) {
                // Set up the queue metadata object in order to track priority, depth, index into the queue list of the
                // backing queue object, and message size. Both index and message size are calculated where priority and
                // depth are copied from the configuration object.
                QueueMetadata& entry = this->m_prioritizedList[currentPriorityIndex];
                entry.priority = queueConfig.entries[entryIndex].priority;
                entry.depth = queueConfig.entries[entryIndex].depth;
                entry.index = entryIndex;
                // Message size is determined by the type of object being stored, which in turn is determined by the
                // index of the entry. Those lower than COM_PORT_COUNT are Fw::ComBuffers and those larger Fw::Buffer.
                entry.msgSize = (entryIndex < COM_PORT_COUNT) ? sizeof(Fw::ComBuffer) : sizeof(Fw::Buffer);
                totalAllocation += static_cast<NATIVE_UINT_TYPE>(entry.depth * entry.msgSize);
                currentPriorityIndex++;
            }
        }
    }
    // Allocate a single chunk of memory from the memory allocator. Memory recover is neither needed nor used.
    bool recoverable = false;
    this->m_allocation = this->m_allocator->allocate(this->m_allocationId, totalAllocation, recoverable);

    // Each of the backing queue objects must be supplied memory to store the queued messages. These data regions are
    // sub-portions of the total allocated data. This memory is passed out by looping through each queue in prioritized
    // order and passing out the memory to each queue's setup method.
    FwSizeType allocationOffset = 0;
    for (FwIndexType i = 0; i < TOTAL_PORT_COUNT; i++) {
        // Get current queue's allocation size and safety check the values
        FwSizeType allocationSize = this->m_prioritizedList[i].depth * this->m_prioritizedList[i].msgSize;
        FW_ASSERT(this->m_prioritizedList[i].index < static_cast<FwIndexType>(FW_NUM_ARRAY_ELEMENTS(this->m_queues)),
                  this->m_prioritizedList[i].index);
        FW_ASSERT((allocationSize + allocationOffset) <= totalAllocation, static_cast<FwAssertArgType>(allocationSize),
                  static_cast<FwAssertArgType>(allocationOffset), static_cast<FwAssertArgType>(totalAllocation));

        // Setup queue's memory allocation, depth, and message size. Setup is skipped for a depth 0 queue
        if (allocationSize > 0) {
            this->m_queues[this->m_prioritizedList[i].index].setup(
                reinterpret_cast<U8*>(this->m_allocation) + allocationOffset, allocationSize,
                this->m_prioritizedList[i].depth, this->m_prioritizedList[i].msgSize);
        }
        allocationOffset += allocationSize;
    }
    // Safety check that all memory was used as expected
    FW_ASSERT(allocationOffset == totalAllocation, static_cast<FwAssertArgType>(allocationOffset),
              static_cast<FwAssertArgType>(totalAllocation));
}
// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void ComQueue::comQueueIn_handler(const FwIndexType portNum, Fw::ComBuffer& data, U32 context) {
#if DEBUG_COMQUEUE
    // printf("[COMQUEUE] Received COM data: port=%u, size=%llu\n", portNum, data.getBuffLength());
#endif
    // Ensure that the port number of comQueueIn is consistent with the expectation
    FW_ASSERT(portNum >= 0 && portNum < COM_PORT_COUNT, portNum);
    (void)this->enqueue(portNum, QueueType::COM_QUEUE, reinterpret_cast<const U8*>(&data), sizeof(Fw::ComBuffer));
}

void ComQueue::buffQueueIn_handler(const FwIndexType portNum, Fw::Buffer& fwBuffer) {
    const FwIndexType queueNum = portNum + COM_PORT_COUNT;
    // Ensure that the port number of buffQueueIn is consistent with the expectation
    FW_ASSERT(portNum >= 0 && portNum < BUFFER_PORT_COUNT, portNum);
    FW_ASSERT(queueNum < TOTAL_PORT_COUNT);
    bool status =
        this->enqueue(queueNum, QueueType::BUFFER_QUEUE, reinterpret_cast<const U8*>(&fwBuffer), sizeof(Fw::Buffer));
    if (!status) {
        this->deallocate_out(portNum, fwBuffer);
    }
}

void ComQueue::comStatusIn_handler(const FwIndexType portNum, Fw::Success& condition) {
    // printf("[COMQUEUE] comStatusIn_handler called with state=%d, status=%d\n", this->m_state, condition.e);

    if (condition.e == Fw::Success::SUCCESS) {
        // printf("[COMQUEUE] Received SUCCESS, changing to READY state\n");
        this->m_state = READY;
        // Add this to see if processQueue is being called
        // printf("[COMQUEUE] About to call processQueue()\n");
        this->processQueue();
        // Add this after processQueue returns
        // printf("[COMQUEUE] processQueue() completed\n");
    }

    switch (this->m_state) {
        case WAITING:
            if (condition.e == Fw::Success::SUCCESS) {
                // printf("[COMQUEUE] Received SUCCESS, changing to READY state\n");
                this->m_state = READY;
                this->processQueue();
            } else {
                // printf("[COMQUEUE] Received FAILURE, remaining in WAITING state\n");
                this->m_state = WAITING;
            }
            break;
        default:
            // printf("[COMQUEUE] Unexpected state: %d\n", this->m_state);
            FW_ASSERT(0, this->m_state);
            break;
    }
}

void ComQueue::run_handler(const FwIndexType portNum, U32 context) {
    // Downlink the high-water marks for the Fw::ComBuffer array types
    ComQueueDepth comQueueDepth;
    for (U32 i = 0; i < comQueueDepth.SIZE; i++) {
        comQueueDepth[i] = static_cast<U32>(this->m_queues[i].get_high_water_mark());
        this->m_queues[i].clear_high_water_mark();
    }
    this->tlmWrite_comQueueDepth(comQueueDepth);

    // Downlink the high-water marks for the Fw::Buffer array types
    BuffQueueDepth buffQueueDepth;
    for (U32 i = 0; i < buffQueueDepth.SIZE; i++) {
        buffQueueDepth[i] = static_cast<U32>(this->m_queues[i + COM_PORT_COUNT].get_high_water_mark());
        this->m_queues[i + COM_PORT_COUNT].clear_high_water_mark();
    }
    this->tlmWrite_buffQueueDepth(buffQueueDepth);
}

// ----------------------------------------------------------------------
// Hook implementations for typed async input ports
// ----------------------------------------------------------------------

void ComQueue::buffQueueIn_overflowHook(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    FW_ASSERT(portNum >= 0 && portNum < BUFFER_PORT_COUNT, portNum);
    this->deallocate_out(portNum, fwBuffer);
}

// ----------------------------------------------------------------------
// Private helper methods
// ----------------------------------------------------------------------

bool ComQueue::enqueue(const FwIndexType queueNum, QueueType queueType, const U8* data, const FwSizeType size) {
    // Enqueue the given message onto the matching queue. When no space is available then emit the queue overflow event,
    // set the appropriate throttle, and move on. Will assert if passed a message for a depth 0 queue.
    const FwSizeType expectedSize = (queueType == QueueType::COM_QUEUE) ? sizeof(Fw::ComBuffer) : sizeof(Fw::Buffer);
    const FwIndexType portNum = queueNum - ((queueType == QueueType::COM_QUEUE) ? 0 : COM_PORT_COUNT);
    bool rvStatus = true;
    FW_ASSERT(expectedSize == size, static_cast<FwAssertArgType>(size), static_cast<FwAssertArgType>(expectedSize));
    FW_ASSERT(portNum >= 0, portNum);
    Fw::SerializeStatus status = this->m_queues[queueNum].enqueue(data, size);
    if (status == Fw::FW_SERIALIZE_NO_ROOM_LEFT) {
        if (!this->m_throttle[queueNum]) {
            this->log_WARNING_HI_QueueOverflow(queueType, static_cast<U32>(portNum));
            this->m_throttle[queueNum] = true;
        }

        rvStatus = false;
    }
    // When the component is already in READY state process the queue to send out the next available message immediately
    if (this->m_state == READY) {
        this->processQueue();
    }

    return rvStatus;
}

void ComQueue::sendComBuffer(Fw::ComBuffer& comBuffer) {
    // printf("[COMQUEUE] sendComBuffer: size=%lu\n", static_cast<unsigned long>(comBuffer.getBuffLength()));

    FW_ASSERT(this->m_state == READY);
    this->comQueueSend_out(0, comBuffer, 0);
    this->m_state = WAITING;

    // printf("[COMQUEUE] Sent ComBuffer and set state to WAITING\n");
}

void ComQueue::sendBuffer(Fw::Buffer& buffer) {
    // printf("[COMQUEUE] sendBuffer: size=%u, data=%p\n", buffer.getSize(), buffer.getData());

    FW_ASSERT(this->m_state == READY);
    this->buffQueueSend_out(0, buffer);
    this->m_state = WAITING;

    // printf("[COMQUEUE] Sent Buffer and set state to WAITING\n");
}

void ComQueue::processQueue() {
    // printf("[COMQUEUE] Process queue called, state=%d\n", this->m_state);

    // Check state first
    if (this->m_state != READY) {
        // printf("[COMQUEUE] Queue not in ready state, skipping processing\n");
        return;
    }

    FwIndexType priorityIndex = 0;

    // Count active queues for debugging
    U32 nonEmptyQueues = 0;
    for (FwIndexType i = 0; i < TOTAL_PORT_COUNT; i++) {
        if (this->m_queues[i].getQueueSize() > 0) {
            nonEmptyQueues++;
        //     printf("[COMQUEUE] Queue %u has %lu items\n", i,
        //            static_cast<unsigned long>(this->m_queues[i].getQueueSize()));
        }
    }
    // printf("[COMQUEUE] Found %u queues with data out of %u total\n", nonEmptyQueues, TOTAL_PORT_COUNT);

    // Walk all the queues in priority order
    for (priorityIndex = 0; priorityIndex < TOTAL_PORT_COUNT; priorityIndex++) {
        QueueMetadata& entry = this->m_prioritizedList[priorityIndex];
        Types::Queue& queue = this->m_queues[entry.index];

        FwSizeType queueSize = queue.getQueueSize();
        if (queueSize == 0) {
            continue;
        }

        // printf("[COMQUEUE] Found data in queue %u (priority %u), size: %lu\n", entry.index, entry.priority,
        //        static_cast<unsigned long>(queueSize));

        // Send out the message based on the type
        if (entry.index < COM_PORT_COUNT) {
            // printf("[COMQUEUE] Sending Com buffer from queue %u\n", entry.index);
            Fw::ComBuffer comBuffer;
            queue.dequeue(reinterpret_cast<U8*>(&comBuffer), sizeof(comBuffer));
            // printf("[COMQUEUE] Dequeued buffer with size %lu\n", static_cast<unsigned long>(comBuffer.getBuffLength()));
            this->sendComBuffer(comBuffer);
        } else {
            // printf("[COMQUEUE] Sending Buffer from queue %u\n", entry.index);
            Fw::Buffer buffer;
            queue.dequeue(reinterpret_cast<U8*>(&buffer), sizeof(buffer));
            // printf("[COMQUEUE] Dequeued buffer with size %u\n", buffer.getSize());
            this->sendBuffer(buffer);
        }

        this->m_throttle[entry.index] = false;

        // printf("[COMQUEUE] Successfully sent data from queue %u\n", entry.index);
        break;
    }
    // printf("[COMQUEUE] processQueue exiting, processed priorityIndex=%u\n", priorityIndex);


    // Add a check after the loop
    if (priorityIndex >= TOTAL_PORT_COUNT) {
        printf("[COMQUEUE] No data was sent from any queue\n");
    }
}
}  // end namespace Svc
