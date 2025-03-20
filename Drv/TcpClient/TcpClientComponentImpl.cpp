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
#include <Fw/Logger/Logger.hpp>  // Optional, for logging
#include <Fw/Types/Assert.hpp>
#include <limits>

namespace Drv {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

TcpClientComponentImpl::TcpClientComponentImpl(const char* const compName)
    : TcpClientComponentBase(compName), m_allocation_size(4096) {}  // Default buffer size

    SocketIpStatus TcpClientComponentImpl::configure(
        const char* hostname, 
        const U16 port,
        const U32 send_timeout_seconds,
        const U32 send_timeout_microseconds,
        const FwSizeType buffer_size) {
        
        // Store hostname and port for logging
        m_hostname_str = hostname;
        m_port = port;
        
        // Existing code
        FW_ASSERT(buffer_size <= std::numeric_limits<U32>::max(), static_cast<FwAssertArgType>(buffer_size));
        m_allocation_size = buffer_size; // Store the buffer size
        
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
    Drv::RecvStatus recvStatus = (status == SOCK_SUCCESS)             ? RecvStatus::RECV_OK
                                 : (status == SOCK_NO_DATA_AVAILABLE) ? RecvStatus::RECV_NO_DATA
                                                                      : RecvStatus::RECV_ERROR;
    this->recv_out(0, buffer, recvStatus);
}

void TcpClientComponentImpl::connected() {
    if (isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
    }
}
bool TcpClientComponentImpl::isStarted() {
    // Use our local tracking variable
    return m_socketStarted;
}

SocketIpStatus TcpClientComponentImpl::startup() {
    // Check our local state
    if (!m_socketStarted) {
        SocketIpStatus status = this->open();
        if (status == SOCK_SUCCESS) {
            m_socketStarted = true;
            // Port was already set in configure()
            
            // Log connection - using proper logging method
            Fw::Logger::log(m_hostname_str.toChar(), m_port);
        }
        return status;
    }
    return SOCK_SUCCESS;
}

void TcpClientComponentImpl::terminate() {
    this->close(); // Close the socket
    m_socketStarted = false; // Update our tracking variable
}

void TcpClientComponentImpl::readLoop() {
    SocketIpStatus status = SOCK_SUCCESS;
    
    // Keep trying to reconnect until the status is good, told to stop, or reconnection is turned off
    do {
        status = this->startup();
        if (status != SOCK_SUCCESS) {
            Fw::Logger::log(
                "[WARNING] Failed to connect to %s:%d with status %d\n",
                m_hostname_str.toChar(),
                m_port,
                status);
            (void)Os::Task::delay(SOCKET_RETRY_INTERVAL);
            continue;
        }
    } while (this->running() && status != SOCK_SUCCESS && this->m_reopen);
    
    // If start up was successful then perform normal operations
    if (this->running() && status == SOCK_SUCCESS) {
        // Perform the nominal read loop
        SocketComponentHelper::readLoop();
    }
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

}  // end namespace Drv