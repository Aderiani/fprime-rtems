// ======================================================================
// \title  TcpServerSocket.cpp
// \author mstarch, [Your Name]
// \brief  cpp file for TcpServerSocket core implementation classes
//
// \copyright
// Copyright 2009-2020, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship acknowledged.
// Updates for RTEMS Copyright 2025, [Your Organization or Name].
// ======================================================================

#include <Drv/Ip/TcpServerSocket.hpp>
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

#include <cstring>
#include <cerrno>
namespace Drv {

TcpServerSocket::TcpServerSocket() : IpSocket() {}

U16 TcpServerSocket::getListenPort() {
    return this->m_port;
}

SocketIpStatus TcpServerSocket::startup(SocketDescriptor& socketDescriptor) {
    PlatformIntType serverFd = -1;
    struct sockaddr_in address;

    // Acquire a socket, or return error
    if ((serverFd = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        Fw::Logger::log("[ERROR] Failed to create TCP server socket: %d\n", errno);
        return SOCK_FAILED_TO_GET_SOCKET;
    }

    // Enable address reuse to avoid "address already in use" errors
    int opt = 1;
    if (::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        Fw::Logger::log("[ERROR] Failed to set SO_REUSEADDR: %d\n", errno);
        ::close(serverFd);
        return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
    }

    // Set up the address port and name
    ::memset(&address, 0, sizeof(address)); // Clear structure for RTEMS compatibility
    address.sin_family = AF_INET;
    address.sin_port = htons(this->m_port);

#if defined TGT_OS_TYPE_VXWORKS || defined TGT_OS_TYPE_DARWIN
    address.sin_len = static_cast<U8>(sizeof(struct sockaddr_in));
#endif

    // Use INADDR_ANY if hostname is nullptr or invalid, otherwise convert hostname
    if (m_hostname[0] == '\0' || IpSocket::addressToIp4(m_hostname, &(address.sin_addr)) != SOCK_SUCCESS) {
        address.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces by default on RTEMS
    }

    // Bind to the address
    if (::bind(serverFd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        Fw::Logger::log("[ERROR] Failed to bind to %s:%hu: %d\n", m_hostname, m_port, errno);
        ::close(serverFd);
        return SOCK_FAILED_TO_BIND;
    }

    // Verify bound port (optional, but kept for consistency)
    socklen_t size = sizeof(address);
    if (::getsockname(serverFd, reinterpret_cast<struct sockaddr*>(&address), &size) == -1) {
        Fw::Logger::log("[ERROR] Failed to read back port: %d\n", errno);
        ::close(serverFd);
        return SOCK_FAILED_TO_READ_BACK_PORT;
    }

    // Listen with a backlog of 1 (single client)
    if (::listen(serverFd, 1) < 0) {
        Fw::Logger::log("[ERROR] Failed to listen on %s:%hu: %d\n", m_hostname, m_port, errno);
        ::close(serverFd);
        return SOCK_FAILED_TO_LISTEN;
    }

    Fw::Logger::log("Listening for single client at %s:%hu\n", m_hostname, m_port);
    FW_ASSERT(serverFd != -1);
    socketDescriptor.serverFd = serverFd;
    this->m_port = ntohs(address.sin_port); // Update port if dynamically assigned
    return SOCK_SUCCESS;
}

void TcpServerSocket::terminate(const SocketDescriptor& socketDescriptor) {
    if (socketDescriptor.serverFd != -1) {
        (void)::close(socketDescriptor.serverFd);
    }
}

SocketIpStatus TcpServerSocket::openProtocol(SocketDescriptor& socketDescriptor) {
    PlatformIntType clientFd = -1;
    PlatformIntType serverFd = socketDescriptor.serverFd;

    // Check if server socket is started
    if (serverFd == -1) {
        Fw::Logger::log("[ERROR] Server not started before open\n");
        return SOCK_NOT_STARTED;
    }

    // Accept a client connection
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    clientFd = ::accept(serverFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);
    if (clientFd < 0) {
        Fw::Logger::log("[ERROR] Failed to accept client: %d\n", errno);
        return SOCK_FAILED_TO_ACCEPT;
    }

    // Setup client send timeouts
    if (IpSocket::setupTimeouts(clientFd) != SOCK_SUCCESS) {
        Fw::Logger::log("[ERROR] Failed to set client socket timeouts\n");
        ::close(clientFd);
        return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    Fw::Logger::log("Accepted client from %s:%hu at %s:%hu\n", clientIp, ntohs(clientAddr.sin_port), m_hostname, m_port);
    socketDescriptor.fd = clientFd;
    return SOCK_SUCCESS;
}

I32 TcpServerSocket::sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    return static_cast<I32>(::send(socketDescriptor.fd, data, size, SOCKET_IP_SEND_FLAGS));
}

I32 TcpServerSocket::recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) {
    I32 size_buf = static_cast<I32>(::recv(socketDescriptor.fd, data, size, SOCKET_IP_RECV_FLAGS));
    return size_buf;
}

} // namespace Drv