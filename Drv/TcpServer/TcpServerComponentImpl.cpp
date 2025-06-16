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
#define DEBUG_TCP_SERVER 1  // Set to 0 to disable debug prints
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

    // // For RTEMS, limit buffer size to avoid overwhelming the queue
    // const FwSizeType max_rtems_buffer = 1024;  // 1KB max for RTEMS
    // if (buffer_size > max_rtems_buffer) {
    //     Fw::Logger::log("WARNING: Reducing buffer size from %lu to %lu for RTEMS compatibility",
    //                     static_cast<unsigned long>(buffer_size), static_cast<unsigned long>(max_rtems_buffer));
    //     buffer_size = max_rtems_buffer;
    // }

    FW_ASSERT(buffer_size <= std::numeric_limits<U32>::max(), static_cast<FwAssertArgType>(buffer_size));
    m_allocation_size = buffer_size;

    (void)m_socket.configure(hostname, port, send_timeout_seconds, send_timeout_microseconds);
    return startup();
}

void TcpServerComponentImpl::sendBuffer(Fw::Buffer buffer, SocketIpStatus status) {


    // For RTEMS, handle potential null buffer cases
    if (buffer.getData() == nullptr) {
        return;
    }

    // Check if this is our special internal buffer that doesn't need deallocation
    U32 context = buffer.getContext();
    U32 mgrId = context >> 16;

    // Special marker for our internal buffers (0xDEAD)
    if (mgrId == 0xDEAD) {
        // Just pass the buffer to recv_out without special handling
        Drv::RecvStatus recvStatus = (status == SOCK_SUCCESS)             ? RecvStatus::RECV_OK
                                     : (status == SOCK_NO_DATA_AVAILABLE) ? RecvStatus::RECV_NO_DATA
                                                                          : RecvStatus::RECV_ERROR;
        this->recv_out(0, buffer, recvStatus);
        return;
    }

    // Regular buffer processing for normally allocated buffers
    Drv::RecvStatus recvStatus = RecvStatus::RECV_ERROR;
    if (status == SOCK_SUCCESS) {
        recvStatus = RecvStatus::RECV_OK;
    } else if (status == SOCK_NO_DATA_AVAILABLE) {
        recvStatus = RecvStatus::RECV_NO_DATA;
    } else {
        recvStatus = RecvStatus::RECV_ERROR;
    }

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
    // Instead of creating buffers directly, request them from the buffer manager
    // This ensures they are properly tracked as allocated

    // First try to get a buffer through the normal allocation port
    if (this->isConnected_allocate_OutputPort(0)) {
        Fw::Buffer allocated = this->allocate_out(0, 1024);
        // Check if allocation was successful (non-zero size)
        if (allocated.getSize() > 0) {
            Fw::Logger::log("TcpServer: Got buffer from allocator, data=%p, size=%u, context=0x%X", allocated.getData(),
                            allocated.getSize(), allocated.getContext());
            return allocated;
        }
    }

    // Fallback to our static buffer pool if allocation fails
    static U8 buffer_pool[4][1024] __attribute__((aligned(8)));
    static int buffer_index = 0;

    // Get next buffer in rotation
    buffer_index = (buffer_index + 1) % 4;

    // Zero the buffer to ensure clean state
    memset(buffer_pool[buffer_index], 0, 1024);

    // Mark this as a special buffer that doesn't need deallocation
    // Use a special context value that won't be mistaken for a normal buffer
    // For example, set manager ID to 0xDEAD (not 200) as a marker
    Fw::Buffer buffer(buffer_pool[buffer_index], 1024);
    buffer.setContext(0xDEAD0000);  // Special value to indicate internal buffer

    Fw::Logger::log("TcpServer: Using internal buffer, data=%p, size=%u, context=0x%X", buffer.getData(),
                    buffer.getSize(), buffer.getContext());

    return buffer;
}

void TcpServerComponentImpl::readLoop() {
    // Simplified RTEMS implementation with direct buffer handling
    #if DEBUG_TCP_SERVER
    // printf("[TCP-SERVER] Starting read loop\n");
    #endif

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

            #if DEBUG_TCP_SERVER
            // printf("[TCP-SERVER] Received data: size=%u\n", size);
            // Print the first few bytes to see what's coming in
            if (size >= 4) {
                // printf("[TCP-SERVER] Received header: 0x%02X 0x%02X 0x%02X 0x%02X\n", 
                    //    recv_buffer[0], recv_buffer[1], recv_buffer[2], recv_buffer[3]);
            }
            #endif
            // Create buffer with correct manager ID
            Fw::Buffer buffer;
            // Try to allocate a buffer of appropriate size
            if (this->isConnected_allocate_OutputPort(0)) {
                buffer = this->allocate_out(0, size);

                // Verify the allocation was successful
                if (buffer.getSize() >= size) {
                    // Copy data into the allocated buffer
                    memcpy(buffer.getData(), recv_buffer, size);

                    // Process the properly allocated buffer
                    Drv::RecvStatus recvStatus = RecvStatus::RECV_OK;
                    this->recv_out(0, buffer, recvStatus);
                    continue;  // Skip to next iteration
                }
            }

            // Fallback if allocation failed: use static buffer but mark as special
            static U8 msg_buffer[1024] __attribute__((aligned(8)));
            if (size <= sizeof(msg_buffer)) {
                memcpy(msg_buffer, recv_buffer, size);

                // Create buffer with special marker
                Fw::Buffer fallbackBuffer(msg_buffer, size);
                fallbackBuffer.setContext(0xDEAD0000);  // Special marker

                Drv::RecvStatus recvStatus = RecvStatus::RECV_OK;
                this->recv_out(0, fallbackBuffer, recvStatus);
            } else if (status == SOCK_DISCONNECTED) {
                this->close();
                Fw::Logger::log("Client disconnected, will reopen");
            } else {
                // Short delay to avoid CPU spinning
                Os::Task::delay(Fw::TimeInterval(0, 50000));  // 50ms
            }
        }
    }
}
// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

// In Drv/TcpServer/TcpServerComponentImpl.cpp, enhance send_handler:

Drv::SendStatus TcpServerComponentImpl::send_handler(const FwIndexType portNum, Fw::Buffer& fwBuffer) {
    // printf("[TCP-SERVER] Send handler called: size=%u, data=%p, context=0x%X\n", 
        //    fwBuffer.getSize(), fwBuffer.getData(), fwBuffer.getContext());
    
    // Print first few bytes of packet for debugging
    if (fwBuffer.getSize() >= 4) {
        // printf("[TCP-SERVER] Packet header: 0x%02X 0x%02X 0x%02X 0x%02X\n", 
            //    fwBuffer.getData()[0], fwBuffer.getData()[1], 
            //    fwBuffer.getData()[2], fwBuffer.getData()[3]);
    }
    
    // Check if socket is opened
    if (!this->isOpened()) {
        // printf("[TCP-SERVER] Socket not opened, returning SEND_RETRY\n");
        return SendStatus::SEND_RETRY;
    }
    
    // Ensure buffer has valid data
    if (fwBuffer.getData() == nullptr || fwBuffer.getSize() == 0) {
        // printf("[TCP-SERVER] Invalid buffer (NULL or zero size)\n");
        return SendStatus::SEND_ERROR;
    }
    
    // Special marker for internal buffers (0xDEAD)
    U32 context = fwBuffer.getContext();
    U32 mgrId = context >> 16;
    bool isInternalBuffer = (mgrId == 0xDEAD);
    
    // Try to send data
    // printf("[TCP-SERVER] Sending %u bytes to socket\n", fwBuffer.getSize());
    Drv::SocketIpStatus status = this->send(fwBuffer.getData(), fwBuffer.getSize());
    // printf("[TCP-SERVER] Send returned status: %d\n", status);
    
    if (status == SOCK_INTERRUPTED_TRY_AGAIN) {
        // printf("[TCP-SERVER] Send interrupted, returning SEND_RETRY\n");
        return SendStatus::SEND_RETRY;
    } else if (status != SOCK_SUCCESS) {
        // printf("[TCP-SERVER] Send error: %d\n", status);
        // Only deallocate if not an internal buffer
        if (!isInternalBuffer && this->isConnected_deallocate_OutputPort(0)) {
            // printf("[TCP-SERVER] Deallocating buffer after error\n");
            deallocate_out(0, fwBuffer);
        }
        return SendStatus::SEND_ERROR;
    }
    
    // Only deallocate if not an internal buffer
    if (!isInternalBuffer && this->isConnected_deallocate_OutputPort(0)) {
        // printf("[TCP-SERVER] Deallocating buffer after successful send\n");
        deallocate_out(0, fwBuffer);
    }
    
    // printf("[TCP-SERVER] Send successful\n");
    return SendStatus::SEND_OK;
}

bool TcpServerComponentImpl::verifyNetworkReady() {
    // Try up to 5 times with increasing delays
    for (int attempt = 1; attempt <= 5; attempt++) {

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

            ::close(test_socket);
            return true;
        }

        ::close(test_socket);
        Os::Task::delay(Fw::TimeInterval(attempt, 0));  // Increasing delay
    }

    Fw::Logger::log("[WARNING] Network not ready after multiple attempts");
    return false;
}
}  // namespace Drv