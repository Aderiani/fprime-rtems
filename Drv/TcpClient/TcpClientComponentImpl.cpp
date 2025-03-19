// ======================================================================
// \title  TcpClientComponentImpl.cpp
// \author mstarch, [Your Name]
// \brief  cpp file for TcpClientComponentImpl component implementation class
//
// \copyright
// Copyright 2009-2020, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// Updates for RTEMS Copyright 2025, [Your Organization or Name].
// ======================================================================

#include <Drv/TcpClient/TcpClientComponentImpl.hpp>
#include <FpConfig.hpp>
#include <limits>
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp> // Optional, for logging

namespace Drv {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

TcpClientComponentImpl::TcpClientComponentImpl(const char* const compName)
    : TcpClientComponentBase(compName), m_allocation_size(4096) {} // Default buffer size

SocketIpStatus TcpClientComponentImpl::configure(const char* hostname,
                                                 const U16 port,
                                                 const U32 send_timeout_seconds,
                                                 const U32 send_timeout_microseconds,
                                                 FwSizeType buffer_size) {
    FW_ASSERT(buffer_size <= std::numeric_limits<U32>::max(), static_cast<FwAssertArgType>(buffer_size));
    m_allocation_size = buffer_size;
    // RTEMS: hostname must be a valid IP or resolvable hostname; network must be initialized
    return m_socket.configure(hostname, port, send_timeout_seconds, send_timeout_microseconds);
}

TcpClientComponentImpl::~TcpClientComponentImpl() {}

// ----------------------------------------------------------------------
// Implementations for socket read task virtual methods
// ----------------------------------------------------------------------

IpSocket& TcpClientComponentImpl::getSocketHandler() {
    return m_socket;
}

Fw::Buffer TcpClientComponentImpl::getBuffer() {
    return allocate_out(0, static_cast<U32>(m_allocation_size));
}

void TcpClientComponentImpl::sendBuffer(Fw::Buffer buffer, SocketIpStatus status) {
    Drv::RecvStatus recvStatus = (status == SOCK_SUCCESS) ? RecvStatus::RECV_OK :
                                 (status == SOCK_NO_DATA_AVAILABLE) ? RecvStatus::RECV_NO_DATA :
                                 RecvStatus::RECV_ERROR;
    this->recv_out(0, buffer, recvStatus);
}

void TcpClientComponentImpl::connected() {
    if (isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
    }
}

bool TcpClientComponentImpl::isStarted() {
    Os::ScopeLock scopedLock(this->m_lock);
    return this->m_socket.isOpened(); // Check if socket is open
}

SocketIpStatus TcpClientComponentImpl::startup() {
    Os::ScopeLock scopedLock(this->m_lock);
    Drv::SocketIpStatus status = SOCK_SUCCESS;
    if (!this->m_socket.isOpened()) {
        SocketDescriptor descriptor;
        status = this->m_socket.open(descriptor);
        if (status != SOCK_SUCCESS) {
            Fw::Logger::log("[WARNING] Failed to connect to server on port %hu with status %d\n",
                            this->m_socket.getPort(), status);
        }
    }
    return status;
}

void TcpClientComponentImpl::terminate() {
    Os::ScopeLock scopedLock(this->m_lock);
    SocketDescriptor descriptor;
    this->m_socket.close(descriptor);
}

void TcpClientComponentImpl::readLoop() {
    Drv::SocketIpStatus status = Drv::SocketIpStatus::SOCK_NOT_STARTED;
    do {
        status = this->startup();
        if (status != SOCK_SUCCESS) {
            Fw::Logger::log("[WARNING] Failed to connect to server on port %hu with status %d\n",
                            this->m_socket.getPort(), status);
            (void)Os::Task::delay(SOCKET_RETRY_INTERVAL);
            continue;
        }
    } while (this->running() && status != SOCK_SUCCESS && this->m_reopen);

    if (this->running() && status == SOCK_SUCCESS) {
        SocketComponentHelper::readLoop();
    }
    this->terminate();
}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

Drv::SendStatus TcpClientComponentImpl::send_handler(const FwIndexType portNum, Fw::Buffer& fwBuffer) {
    Drv::SocketIpStatus status = this->send(fwBuffer.getData(), fwBuffer.getSize());
    if (status == SOCK_INTERRUPTED_TRY_AGAIN) {
        return SendStatus::SEND_RETRY;
    } else if (status != SOCK_SUCCESS) {
        deallocate_out(0, fwBuffer);
        return SendStatus::SEND_ERROR;
    }
    deallocate_out(0, fwBuffer);
    return SendStatus::SEND_OK;
}

} // end namespace Drv