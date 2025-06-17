// ======================================================================
// \title  IpSocket.cpp
// \author mstarch, crsmith, [Your Name]
// \brief  cpp file for IpSocket core implementation classes
//
// \copyright
// Copyright 2009-2020, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// Updates for RTEMS Copyright 2025, [Your Organization or Name].
// ======================================================================

#include <cstring>
#include <cerrno>
#include <Drv/Ip/IpSocket.hpp>
#include <Fw/Types/Assert.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/StringUtils.hpp>
#include <sys/time.h>
#include <Fw/Logger/Logger.hpp>

#ifdef TGT_OS_TYPE_VXWORKS
#include <socket.h>
#include <inetLib.h>
#include <fioLib.h>
#include <hostLib.h>
#include <ioLib.h>
#include <vxWorks.h>
#include <sockLib.h>
#include <fioLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <errnoLib.h>
#elif defined TGT_OS_TYPE_LINUX || defined TGT_OS_TYPE_DARWIN || defined __rtems__
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#error OS not supported for IP Socket Communications
#endif

namespace Drv {

IpSocket::IpSocket() : m_timeoutSeconds(0), m_timeoutMicroseconds(0), m_port(0) {
    ::memset(m_hostname, 0, sizeof(m_hostname));
}

SocketIpStatus IpSocket::configure(const char* const hostname, const U16 port, const U32 timeout_seconds, const U32 timeout_microseconds) {
    FW_ASSERT(timeout_microseconds < 1000000, static_cast<FwAssertArgType>(timeout_microseconds));
    FW_ASSERT(this->isValidPort(port), static_cast<FwAssertArgType>(port));
    FW_ASSERT(hostname != nullptr);
    this->m_timeoutSeconds = timeout_seconds;
    this->m_timeoutMicroseconds = timeout_microseconds;
    this->m_port = port;
    (void) Fw::StringUtils::string_copy(this->m_hostname, hostname, static_cast<FwSizeType>(SOCKET_MAX_HOSTNAME_SIZE));
    return SOCK_SUCCESS;
}

bool IpSocket::isValidPort(U16 port) {
    return true; // RTEMS supports full port range (0-65535)
}

SocketIpStatus IpSocket::setupTimeouts(PlatformIntType socketFd) {
    #ifdef TGT_OS_TYPE_VXWORKS
        // No timeouts set on VxWorks
        return SOCK_SUCCESS;
    #elif defined(__rtems__)
        // RTEMS compatible timeout handling - simpler approach
        if (this->m_timeoutSeconds == 0 && this->m_timeoutMicroseconds == 0) {
            return SOCK_SUCCESS; // No timeout requested
        }
        
        // Only set socket option if non-zero timeout is requested
        // This avoids potential EOPNOTSUPP errors in RTEMS
        #ifdef SO_SNDTIMEO
        struct timeval timeout;
        timeout.tv_sec = static_cast<time_t>(this->m_timeoutSeconds);
        timeout.tv_usec = static_cast<suseconds_t>(this->m_timeoutMicroseconds);
        
        // Try to set timeout, but continue even if it fails
        if (setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            Fw::Logger::log("[WARNING] Failed to set socket send timeout: %d\n", errno);
            // Don't return error - just continue without timeout
        }
        #endif
        return SOCK_SUCCESS;
    #else // Linux, Darwin
        struct timeval timeout;
        timeout.tv_sec = static_cast<time_t>(this->m_timeoutSeconds);
        timeout.tv_usec = static_cast<suseconds_t>(this->m_timeoutMicroseconds);
        if (setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0) {
            return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
        }
        return SOCK_SUCCESS;
    #endif
    }

SocketIpStatus IpSocket::addressToIp4(const char* address, void* ip4) {
    FW_ASSERT(address != nullptr);
    FW_ASSERT(ip4 != nullptr);
#ifdef TGT_OS_TYPE_VXWORKS
    NATIVE_INT_TYPE ip = inet_addr(address);
    if (ip == ERROR) {
        return SOCK_INVALID_IP_ADDRESS;
    }
    *reinterpret_cast<unsigned long*>(ip4) = ip;
#else // Linux, Darwin, RTEMS
    if (!::inet_pton(AF_INET, address, ip4)) {
        return SOCK_INVALID_IP_ADDRESS;
    }
#endif
    return SOCK_SUCCESS;
}

void IpSocket::close(const SocketDescriptor& socketDescriptor) {
    (void)::close(socketDescriptor.fd);
}

void IpSocket::shutdown(const SocketDescriptor& socketDescriptor) {
    errno = 0;
    PlatformIntType status = ::shutdown(socketDescriptor.fd, SHUT_RDWR);
    if (status != 0) {
        this->close(socketDescriptor);
    }
}

SocketIpStatus IpSocket::open(SocketDescriptor& socketDescriptor) {
    SocketIpStatus status = SOCK_SUCCESS;
    errno = 0;
    status = this->openProtocol(socketDescriptor);
    if (status != SOCK_SUCCESS) {
        socketDescriptor.fd = -1;
        return status;
    }
    return status;
}

SocketIpStatus IpSocket::send(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    U32 total = 0;
    I32 sent = 0;
    for (U32 i = 0; (i < SOCKET_MAX_ITERATIONS) && (total < size); i++) {
        errno = 0;

        sent = this->sendProtocol(socketDescriptor, data + total, size - total);
        if (((sent == -1) && (errno == EINTR)) || (sent == 0)) {
            continue;
        } else if ((sent == -1) && ((errno == EBADF) || (errno == ECONNRESET))) {
            return SOCK_DISCONNECTED;
        } else if (sent == -1) {
            Fw::Logger::log("[IpSocket::send] SOCK_SEND_ERROR, fd: %d, errno: %d (%s)\n", socketDescriptor.fd, errno, strerror(errno));
            return SOCK_SEND_ERROR;
        }
        FW_ASSERT(sent > 0, sent);
        total += static_cast<U32>(sent);
    }
    if (total < size) {
        Fw::Logger::log("[IpSocket::send] SOCK_INTERRUPTED_TRY_AGAIN after max iterations, fd: %d, sent %u of %u bytes, last errno during loop: %d (%s)\n", socketDescriptor.fd, total, size, errno, strerror(errno)); // Added strerror
        return SOCK_INTERRUPTED_TRY_AGAIN;
    }
    FW_ASSERT(total == size, static_cast<FwAssertArgType>(total), static_cast<FwAssertArgType>(size));
    return SOCK_SUCCESS;
}

SocketIpStatus IpSocket::recv(const SocketDescriptor& socketDescriptor, U8* data, U32& req_read) {
    I32 size = 0;
    for (U32 i = 0; (i < SOCKET_MAX_ITERATIONS) && (size <= 0); i++) {
        errno = 0;
        size = this->recvProtocol(socketDescriptor, data, req_read);
        if ((size == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            req_read = 0;
            return SOCK_NO_DATA_AVAILABLE;
        } else if ((size == -1) && (errno == EINTR)) {
            continue;
        } else if (size == 0 || ((size == -1) && ((errno == ECONNRESET) || (errno == EBADF)))) {
            req_read = static_cast<U32>(size);
            return SOCK_DISCONNECTED;
        } else if (size == -1) {
            req_read = static_cast<U32>(size);
            return SOCK_READ_ERROR;
        }
    }
    req_read = static_cast<U32>(size);
    if (size == -1) {
        return SOCK_INTERRUPTED_TRY_AGAIN;
    }
    return SOCK_SUCCESS;
}

} // namespace Drv