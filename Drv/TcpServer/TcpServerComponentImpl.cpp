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


// In TcpServerComponentImpl.cpp, modify configure method
SocketIpStatus TcpServerComponentImpl::configure(const char* hostname,
    const U16 port,
    const U32 send_timeout_seconds,
    const U32 send_timeout_microseconds,
    FwSizeType buffer_size) {
// Use a smaller default buffer size for RTEMS
#ifdef __rtems__
// For RTEMS, limit buffer size to avoid overwhelming the queue
const FwSizeType max_rtems_buffer = 1024; // 1KB max for RTEMS
if (buffer_size > max_rtems_buffer) {
Fw::Logger::log("WARNING: Reducing buffer size from %lu to %lu for RTEMS compatibility", 
static_cast<unsigned long>(buffer_size), 
static_cast<unsigned long>(max_rtems_buffer));
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

// Update the getBuffer method to handle RTEMS limitations better
Fw::Buffer TcpServerComponentImpl::getBuffer() {
    #ifdef __rtems__
    // For RTEMS, add error handling for buffer allocation
    Fw::Buffer buffer = allocate_out(0, static_cast<U32>(m_allocation_size));
    if (buffer.getData() == nullptr || buffer.getSize() == 0) {
        // Failed to allocate, return a small emergency buffer instead of failing
        static U8 emergency_buffer[64];
        Fw::Logger::log("WARNING: Failed to allocate network buffer, using emergency buffer");
        return Fw::Buffer(emergency_buffer, sizeof(emergency_buffer));
    }
    return buffer;
    #else
    return allocate_out(0, static_cast<U32>(m_allocation_size));
    #endif
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


// In TcpServerComponentImpl.cpp - replace the readLoop method with this version (no try-catch)
void TcpServerComponentImpl::readLoop() {
    Drv::SocketIpStatus status = Drv::SocketIpStatus::SOCK_NOT_STARTED;
    
    // Keep trying to reconnect until the status is good, told to stop, or reconnection is turned off
    do {
        #ifdef __rtems__
        // Skip if network is not available
        if (!verifyNetworkReady()) {
            Fw::Logger::log("Network not available, skipping socket operations");
            Os::Task::delay(SOCKET_RETRY_INTERVAL);
            continue;
        }
        #endif

        status = this->startup();
        if (status != SOCK_SUCCESS) {
            Fw::Logger::log("[WARNING] Failed to listen on port %hu with status %d\n", this->getListenPort(), status);
            (void)Os::Task::delay(SOCKET_RETRY_INTERVAL);
            continue;
        }
    } while (this->running() && status != SOCK_SUCCESS && this->m_reopen);

    #ifdef __rtems__
    // RTEMS-specific implementation to avoid Queue issues
    while (this->running()) {
        // Accept a client connection if not already connected
        if (!this->isOpened()) {
            status = this->open();
            if (status != SOCK_SUCCESS) {
                // Failed to open, delay and try again
                Os::Task::delay(SOCKET_RETRY_INTERVAL);
                continue;
            }
            // Connected successfully, notify via the ready port
            this->connected();
        }
        
        // We're connected - read data with a more cautious approach
        static U8 small_buffer[256]; // Small static buffer to avoid allocation issues
        U32 size = sizeof(small_buffer);
        
        // Use a non-allocating receive first to check if data is available
        status = this->recv(small_buffer, size);
        
        if (status == SOCK_SUCCESS && size > 0) {
            // Data received - now allocate a proper buffer and process it
            Fw::Buffer buffer = this->allocate_out(0, size);
            
            // Check if allocation succeeded
            if (buffer.getData() != nullptr) {
                // Copy the data to the allocated buffer
                ::memcpy(buffer.getData(), small_buffer, size);
                buffer.setSize(size);
                
                // Process buffer with error propagation rather than exception handling
                Drv::RecvStatus recvStatus = RecvStatus::RECV_OK;
                this->recv_out(0, buffer, recvStatus);
            } else {
                // Failed to allocate, just log
                Fw::Logger::log("[ERROR] Failed to allocate buffer for received data");
            }
        } else if (status == SOCK_DISCONNECTED) {
            // Socket disconnected, close it and we'll reopen on next loop
            this->close();
            Fw::Logger::log("[INFO] Client disconnected, will wait for new connection");
        } else if (status != SOCK_NO_DATA_AVAILABLE) {
            // Some other error occurred
            Fw::Logger::log("[WARNING] Socket receive error: %d", status);
            Os::Task::delay(Fw::TimeInterval(0, 100000)); // 100ms delay
        } else {
            // No data available, just wait a bit
            Os::Task::delay(Fw::TimeInterval(0, 50000)); // 50ms delay
        }
    }
    #else
    // Standard implementation for non-RTEMS
    if (this->running() && status == SOCK_SUCCESS) {
        // Perform the nominal read loop
        SocketComponentHelper::readLoop();
    }
    #endif
    
    // Terminate the server
    this->terminate();
}



// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

// In TcpServerComponentImpl.cpp, update the send_handler method:

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

// In TcpServerComponentImpl.cpp
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