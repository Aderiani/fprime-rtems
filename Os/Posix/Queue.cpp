// ======================================================================
// \title Os/Posix/Queue.cpp
// \brief posix implementation for Os::Queue
// ======================================================================
#include "Queue.hpp"

namespace Os {
namespace Posix {
namespace Queue {

QueueInterface::Status PosixQueue::create(const Fw::StringBase& name, FwSizeType depth, FwSizeType messageSize) {
    return QueueInterface::Status::UNKNOWN_ERROR;
}

QueueInterface::Status PosixQueue::send(const U8* buffer,
                                        FwSizeType size,
                                        FwQueuePriorityType priority,
                                        QueueInterface::BlockingType blockType) {
    return QueueInterface::Status::UNINITIALIZED;
}

QueueInterface::Status PosixQueue::receive(U8* destination,
                                           FwSizeType capacity,
                                           QueueInterface::BlockingType blockType,
                                           FwSizeType& actualSize,
                                           FwQueuePriorityType& priority) {
    return QueueInterface::Status::UNINITIALIZED;
}

FwSizeType PosixQueue::getMessagesAvailable() const {
    return 0;
}

FwSizeType PosixQueue::getMessageHighWaterMark() const {
    return 0;
}

QueueHandle* PosixQueue::getHandle() {
    return &this->m_handle;
}

}  // namespace Queue
}  // namespace Posix
}  // namespace Os
