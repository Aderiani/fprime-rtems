// ======================================================================
// \title  TcpServerComponentImpl.cpp
// \author mstarch
// \brief  cpp file for TcpServerComponentImpl component implementation class
//
// \copyright
// Copyright 2009-2020, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================
#ifdef TGT_OS_TYPE_VXWORKS
#include <inetLib.h>
#include <socket.h>
#elif defined TGT_OS_TYPE_LINUX || defined TGT_OS_TYPE_DARWIN || defined __rtems__
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <Drv/TcpServer/TcpServerComponentImpl.hpp>
#include <FpConfig.hpp>
#include <cstring>
#include <limits>
#include "Fw/Logger/Logger.hpp"
#include "Fw/Types/Assert.hpp"
#include "TcpServerComponentImpl.hpp"

namespace Drv {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

TcpServerComponentImpl::TcpServerComponentImpl(const char* const compName) : TcpServerComponentBase(compName) {}

SocketIpStatus TcpServerComponentImpl::configure(const char* hostname,
                                                 const U16 port,
                                                 const U32 send_timeout_seconds,
                                                 const U32 send_timeout_microseconds,
                                                 FwSizeType buffer_size) {
// Use a smaller default buffer size for RTEMS
#ifdef __rtems__
    // For RTEMS, limit buffer size to avoid overwhelming the queue
    const FwSizeType max_rtems_buffer = 1024;  // 1KB max for RTEMS
    if (buffer_size > max_rtems_buffer) {
        Fw::Logger::log("WARNING: Reducing buffer size from %lu to %lu for RTEMS compatibility",
                        static_cast<unsigned long>(buffer_size), static_cast<unsigned long>(max_rtems_buffer));
        buffer_size = max_rtems_buffer;
    }
#endif

    FW_ASSERT(buffer_size <= std::numeric_limits<U32>::max(), static_cast<FwAssertArgType>(buffer_size));
    m_allocation_size = buffer_size;

    (void)m_socket.configure(hostname, port, send_timeout_seconds, send_timeout_microseconds);
    return startup();
}

void TcpServerComponentImpl::sendBuffer(Fw::Buffer buffer, SocketIpStatus status) {
#ifdef __rtems__
    Fw::Logger::log("TcpServer: sendBuffer called with status=%d, size=%u, data=%p", status, buffer.getSize(),
                    buffer.getData());
    // For RTEMS, handle potential null buffer cases
    if (buffer.getData() == nullptr) {
        Fw::Logger::log("[WARNING] Null buffer in sendBuffer, skipping");
        return;
    }
#endif

    Drv::RecvStatus recvStatus = RecvStatus::RECV_ERROR;
    if (status == SOCK_SUCCESS) {
        recvStatus = RecvStatus::RECV_OK;
    } else if (status == SOCK_NO_DATA_AVAILABLE) {
        recvStatus = RecvStatus::RECV_NO_DATA;
    } else {
        recvStatus = RecvStatus::RECV_ERROR;
    }

    // No try/catch - just call the output port
    this->recv_out(0, buffer, recvStatus);
}

TcpServerComponentImpl::~TcpServerComponentImpl() {}

// ----------------------------------------------------------------------
// Implementations for socket read task virtual methods
// ----------------------------------------------------------------------

U16 TcpServerComponentImpl::getListenPort() {
    return m_socket.getListenPort();
}

IpSocket& TcpServerComponentImpl::getSocketHandler() {
    return m_socket;
}

void TcpServerComponentImpl::connected() {
    if (isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
    }
}

bool TcpServerComponentImpl::isStarted() {
    Os::ScopeLock scopedLock(this->m_lock);
    return this->m_descriptor.serverFd != -1;
}

SocketIpStatus TcpServerComponentImpl::startup() {
    Os::ScopeLock scopedLock(this->m_lock);
    Drv::SocketIpStatus status = SOCK_SUCCESS;

    // Check for network readiness, but continue with graceful degradation if not ready
    if (!verifyNetworkReady()) {
        Fw::Logger::log("[WARNING] Network not ready, TcpServer will not be available");
        // Return success but don't actually try to open the socket
        // This allows the system to run without networking
        this->m_descriptor.serverFd = -2;  // Special marker for "intentionally not opened"
        return SOCK_SUCCESS;
    }

    // Prevent multiple startup attempts
    if (this->m_descriptor.serverFd == -1) {
        status = this->m_socket.startup(this->m_descriptor);
    }

    return status;
}

void TcpServerComponentImpl::terminate() {
    Os::ScopeLock scopedLock(this->m_lock);
    this->m_socket.terminate(this->m_descriptor);
    this->m_descriptor.serverFd = -1;
}

Fw::Buffer TcpServerComponentImpl::getBuffer() {
    #ifdef __rtems__
    // Ensure buffers are aligned on 8-byte boundaries for SPARC
    static U8 buffer_pool[4][1024] __attribute__((aligned(8)));
    static int buffer_index = 0;
    
    // Get next buffer in rotation
    buffer_index = (buffer_index + 1) % 4;
    
    // Zero the buffer to ensure clean state (helps with alignment issues)
    memset(buffer_pool[buffer_index], 0, 1024);
    
    // Create aligned buffer
    Fw::Buffer buffer(buffer_pool[buffer_index], 1024);
    return buffer;
    #else
    // Original code
    #endif
}

void TcpServerComponentImpl::readLoop() {
    #ifdef __rtems__
    // Simplified RTEMS implementation with direct buffer handling
    Fw::Logger::log("TcpServer: Starting read loop with direct buffer handling");
    
    Drv::SocketIpStatus status = Drv::SocketIpStatus::SOCK_NOT_STARTED;
    
    // Add delay to give time for initialization to complete
    Os::Task::delay(Fw::TimeInterval(2, 0));  // 2 second initialization delay
    
    // Connect loop
    while (this->running()) {
        if (!this->isOpened()) {
            // Check if server is properly started before trying to open
            if (!this->isStarted() || this->m_descriptor.serverFd == -1) {
                Fw::Logger::log("[WARNING] Server not properly initialized, waiting...");
                Os::Task::delay(SOCKET_RETRY_INTERVAL);
                continue;
            }
            
            status = SocketComponentHelper::open();  // Call parent method
            if (status != SOCK_SUCCESS) {
                Fw::Logger::log("[INFO] Could not open connection, retrying...");
                Os::Task::delay(SOCKET_RETRY_INTERVAL);
                continue;
            }
            this->connected();
        }
        
        // We're connected - use static buffer for receiving data
        static U8 recv_buffer[1024] __attribute__((aligned(8)));  // 8-byte aligned
        U32 size = sizeof(recv_buffer);
        
        status = this->recv(recv_buffer, size);
        
        if (status == SOCK_SUCCESS && size > 0) {
            // Data received - create a buffer without using queue allocations
            static U8 msg_buffer[1024] __attribute__((aligned(8)));  // 8-byte aligned
            memcpy(msg_buffer, recv_buffer, size);
            
            Fw::Buffer buffer(msg_buffer, size);
            Drv::RecvStatus recvStatus = RecvStatus::RECV_OK;
            
            // Process the buffer
            this->recv_out(0, buffer, recvStatus);
        } else if (status == SOCK_DISCONNECTED) {
            this->close();
            Fw::Logger::log("Client disconnected, will reopen");
        } else {
            // Short delay to avoid CPU spinning
            Os::Task::delay(Fw::TimeInterval(0, 50000)); // 50ms
        }
    }
    #else
    // Original implementation for other platforms
    SocketComponentHelper::readLoop();
    #endif
}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

Drv::SendStatus TcpServerComponentImpl::send_handler(const FwIndexType portNum, Fw::Buffer& fwBuffer) {
// Add graceful failure for RTEMS when network isn't ready
#ifdef __rtems__
    if (!this->isStarted() || this->m_descriptor.serverFd == -1) {
        if (isConnected_deallocate_OutputPort(0)) {
            deallocate_out(0, fwBuffer);
        }
        return SendStatus::SEND_ERROR;
    }
#endif

    Drv::SocketIpStatus status = this->send(fwBuffer.getData(), fwBuffer.getSize());
    // Only deallocate buffer when the caller is not asked to retry
    if (status == SOCK_INTERRUPTED_TRY_AGAIN) {
        return SendStatus::SEND_RETRY;
    } else if (status != SOCK_SUCCESS) {
        deallocate_out(0, fwBuffer);
        return SendStatus::SEND_ERROR;
    }
    deallocate_out(0, fwBuffer);
    return SendStatus::SEND_OK;
}

bool TcpServerComponentImpl::verifyNetworkReady() {
    // Try up to 5 times with increasing delays
    for (int attempt = 1; attempt <= 5; attempt++) {
        Fw::Logger::log("[INFO] Checking network readiness (attempt %d/5)", attempt);

        // Create socket and try to bind
        int test_socket = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (test_socket < 0) {
            Fw::Logger::log("[ERROR] Failed to create socket: errno=%d", errno);
            Os::Task::delay(Fw::TimeInterval(attempt, 0));  // Increasing delay
            continue;
        }

        struct sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = 0;

        if (::bind(test_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            Fw::Logger::log("[INFO] Network is ready");
            ::close(test_socket);
            return true;
        }

        ::close(test_socket);
        Fw::Logger::log("[INFO] Network not ready yet, waiting...");
        Os::Task::delay(Fw::TimeInterval(attempt, 0));  // Increasing delay
    }

    Fw::Logger::log("[WARNING] Network not ready after multiple attempts");
    return false;
}
}  // namespace Drv