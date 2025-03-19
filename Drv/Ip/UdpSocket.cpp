// ======================================================================
// \title  UdpSocket.cpp
// \author mstarch, [Your Name]
// \brief  cpp file for UdpSocket core implementation classes
//
// \copyright
// Copyright 2009-2020, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// Updates for RTEMS Copyright 2025, [Your Organization or Name].
// ======================================================================

#include <Drv/Ip/UdpSocket.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/StringUtils.hpp> // For string_copy
#include <FpConfig.hpp>

#ifdef TGT_OS_TYPE_VXWORKS
    #include <socket.h>
    #include <inetLib.h>
    #include <fioLib.h>
    #include <hostLib.h>
    #include <ioLib.h>
    #include <vxWorks.h>
    #include <sockLib.h>
    #include <taskLib.h>
    #include <sysLib.h>
    #include <errnoLib.h>
    #include <cstring>
#elif defined TGT_OS_TYPE_LINUX || defined TGT_OS_TYPE_DARWIN || defined __rtems__
    #include <sys/socket.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
#else
    #error OS not supported for IP Socket Communications
#endif

#include <cstdio>
#include <cstring>
#include <cerrno>

namespace Drv {

UdpSocket::UdpSocket() : IpSocket(), m_state(nullptr), m_recv_port(0) {
    ::memset(m_recv_hostname, 0, sizeof(m_recv_hostname));
}

UdpSocket::~UdpSocket() {
    // m_state cleanup would go here if allocated, but it's not used in this implementation
}

SocketIpStatus UdpSocket::configure(const char* hostname, const U16 port, const U32 send_timeout_seconds,
                                   const U32 send_timeout_microseconds) {
    FW_ASSERT(0); // configure() is disabled per header
    return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
}

SocketIpStatus UdpSocket::configureSend(const char* hostname, const U16 port, const U32 send_timeout_seconds,
                                        const U32 send_timeout_microseconds) {
    FW_ASSERT(this->isValidPort(port), static_cast<FwAssertArgType>(port));
    FW_ASSERT(hostname != nullptr);
    this->m_timeoutSeconds = send_timeout_seconds;
    this->m_timeoutMicroseconds = send_timeout_microseconds;
    this->m_port = port; // Send port stored in IpSocket's m_port
    (void) Fw::StringUtils::string_copy(this->m_hostname, hostname, sizeof(m_hostname));
    return SOCK_SUCCESS;
}

SocketIpStatus UdpSocket::configureRecv(const char* hostname, const U16 port) {
    FW_ASSERT(this->isValidPort(port), static_cast<FwAssertArgType>(port));
    this->m_recv_port = port; // Receive port stored separately
    if (hostname != nullptr) {
        (void) Fw::StringUtils::string_copy(this->m_recv_hostname, hostname, sizeof(m_recv_hostname));
    } else {
        ::memset(m_recv_hostname, 0, sizeof(m_recv_hostname)); // Clear if null
    }
    return SOCK_SUCCESS;
}

SocketIpStatus UdpSocket::bind(const PlatformIntType fd) {
    struct sockaddr_in recvAddr;
    ::memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(this->m_recv_port);

#if defined TGT_OS_TYPE_VXWORKS || defined TGT_OS_TYPE_DARWIN
    recvAddr.sin_len = static_cast<U8>(sizeof(struct sockaddr_in));
#endif

    // Use INADDR_ANY if m_recv_hostname is empty, otherwise convert it
    if (m_recv_hostname[0] == '\0' || IpSocket::addressToIp4(m_recv_hostname, &(recvAddr.sin_addr)) != SOCK_SUCCESS) {
        recvAddr.sin_addr.s_addr = INADDR_ANY; // Default to all interfaces
    }

    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&recvAddr), sizeof(recvAddr)) < 0) {
        Fw::Logger::log("[ERROR] Failed to bind UDP socket to %s:%hu: %d\n", m_recv_hostname, m_recv_port, errno);
        return SOCK_FAILED_TO_CONNECT;
    }
    return SOCK_SUCCESS;
}

SocketIpStatus UdpSocket::openProtocol(SocketDescriptor& socketDescriptor) {
    NATIVE_INT_TYPE socketFd = -1;

    if ((socketFd = ::socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        Fw::Logger::log("[ERROR] Failed to create UDP socket: %d\n", errno);
        return SOCK_FAILED_TO_GET_SOCKET;
    }

    // Bind if receive port is configured
    if (this->m_recv_port != 0) {
        SocketIpStatus status = this->bind(socketFd);
        if (status != SOCK_SUCCESS) {
            ::close(socketFd);
            return status;
        }
    }

    // Apply timeouts (RTEMS supports SO_SNDTIMEO)
    if (IpSocket::setupTimeouts(socketFd) != SOCK_SUCCESS) {
        ::close(socketFd);
        Fw::Logger::log("[ERROR] Failed to set UDP socket timeouts\n");
        return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
    }

    socketDescriptor.fd = socketFd;
    Fw::Logger::log("UDP socket opened%s%hu\n",
                    (m_recv_port != 0) ? " for receiving on port " : "",
                    m_recv_port);
    return SOCK_SUCCESS;
}

I32 UdpSocket::sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    if (this->m_port == 0 || m_hostname[0] == '\0') {
        return -1; // Send not configured
    }

    struct sockaddr_in sendAddr;
    ::memset(&sendAddr, 0, sizeof(sendAddr));
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(this->m_port);

#if defined TGT_OS_TYPE_VXWORKS || defined TGT_OS_TYPE_DARWIN
    sendAddr.sin_len = static_cast<U8>(sizeof(struct sockaddr_in));
#endif

    if (IpSocket::addressToIp4(m_hostname, &(sendAddr.sin_addr)) != SOCK_SUCCESS) {
        Fw::Logger::log("[ERROR] Invalid send hostname: %s\n", m_hostname);
        return -1;
    }

    return static_cast<I32>(::sendto(socketDescriptor.fd, data, size, SOCKET_IP_SEND_FLAGS,
                                     reinterpret_cast<struct sockaddr*>(&sendAddr), sizeof(sendAddr)));
}

I32 UdpSocket::recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) {
    struct sockaddr_in senderAddr;
    socklen_t addrLen = sizeof(senderAddr);
    return static_cast<I32>(::recvfrom(socketDescriptor.fd, data, size, SOCKET_IP_RECV_FLAGS,
                                       reinterpret_cast<struct sockaddr*>(&senderAddr), &addrLen));
}

U16 UdpSocket::getRecvPort() {
    return m_recv_port;
}

} // namespace Drv