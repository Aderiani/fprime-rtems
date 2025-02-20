// ======================================================================
// \title Os/Posix/Queue.cpp
// \brief posix implementation for Os::Queue
// ======================================================================
#include "Queue.hpp"
#include <cerrno>
#include <cstdio>

namespace Os {
namespace Posix {
namespace Queue {

QueueInterface::Status PosixQueue::create(const Fw::StringBase& name, FwSizeType depth, FwSizeType messageSize) {
    struct mq_attr attr{};
    attr.mq_maxmsg = depth;
    attr.mq_msgsize = messageSize;

    this->m_handle.m_queue = mq_open(name.toChar(), O_CREAT | O_RDWR, 0, &attr);
    if (this->m_handle.m_queue == this->m_handle.PosixQueueHandle::INVALID_QUEUE) {
        return QueueInterface::Status::UNINITIALIZED;
    }
    this->m_handle.m_queueName = name;
    return QueueInterface::Status::OP_OK;
}
PosixQueue::~PosixQueue() {
    (void)mq_close(this->m_handle.m_queue);
    mq_unlink(this->m_handle.m_queueName.toChar());
}

QueueInterface::Status PosixQueue::send(const U8* buffer,
                                        FwSizeType size,
                                        FwQueuePriorityType priority,
                                        QueueInterface::BlockingType blockType) {
    if (this->m_handle.m_queue == this->m_handle.PosixQueueHandle::INVALID_QUEUE) {
        return QueueInterface::Status::UNINITIALIZED;
    }
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
