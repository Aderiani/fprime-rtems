// ======================================================================
// \title  TcpClientSocket.cpp
// \author mstarch, [Your Name]
// \brief  cpp file for TcpClientSocket core implementation classes
//
// \copyright
// Copyright 2009-2020, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// Updates for RTEMS Copyright 2025, [Your Organization or Name].
// ======================================================================

#include <Drv/Ip/TcpClientSocket.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>
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

TcpClientSocket::TcpClientSocket() : IpSocket() {}

bool TcpClientSocket::isValidPort(U16 port) {
    return port != 0; // RTEMS supports standard port range
}

SocketIpStatus TcpClientSocket::openProtocol(SocketDescriptor& socketDescriptor) {
    NATIVE_INT_TYPE socketFd = -1;
    struct sockaddr_in address;

    // Acquire a socket, or return error
    if ((socketFd = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        Fw::Logger::log("[ERROR] Failed to create TCP client socket: %d\n", errno);
        return SOCK_FAILED_TO_GET_SOCKET;
    }

    // Set up the address port and name
    ::memset(&address, 0, sizeof(address)); // Clear structure for RTEMS compatibility
    address.sin_family = AF_INET;
    address.sin_port = htons(this->m_port);

#if defined TGT_OS_TYPE_VXWORKS || defined TGT_OS_TYPE_DARWIN
    address.sin_len = static_cast<U8>(sizeof(struct sockaddr_in));
#endif

    // Convert IP address
    if (IpSocket::addressToIp4(m_hostname, &(address.sin_addr)) != SOCK_SUCCESS) {
        ::close(socketFd);
        Fw::Logger::log("[ERROR] Invalid IP address: %s\n", m_hostname);
        return SOCK_INVALID_IP_ADDRESS;
    }

    // Apply timeouts (RTEMS supports SO_SNDTIMEO)
    if (IpSocket::setupTimeouts(socketFd) != SOCK_SUCCESS) {
        ::close(socketFd);
        Fw::Logger::log("[ERROR] Failed to set socket timeouts\n");
        return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
    }

    // Connect to the server
    if (::connect(socketFd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        Fw::Logger::log("[ERROR] Failed to connect to %s:%hu: %d\n", m_hostname, m_port, errno);
        ::close(socketFd);
        return SOCK_FAILED_TO_CONNECT;
    }

    socketDescriptor.fd = socketFd;
    Fw::Logger::log("Connected to %s:%hu as a TCP client\n", m_hostname, m_port);
    return SOCK_SUCCESS;
}

I32 TcpClientSocket::sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    return static_cast<I32>(::send(socketDescriptor.fd, data, size, SOCKET_IP_SEND_FLAGS));
}

I32 TcpClientSocket::recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) {
    return static_cast<I32>(::recv(socketDescriptor.fd, data, size, SOCKET_IP_RECV_FLAGS));
}

} // namespace Drv