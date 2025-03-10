// ======================================================================
// \title  RtemsTcpServerAdapter.cpp
// \author fprime-community
// \brief  RTEMS specific implementation for Drv::TcpServerSocket
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include "RtemsTcpServerAdapter.hpp"
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

namespace Drv {
namespace RTEMS {

RtemsTcpServerSocket::RtemsTcpServerSocket() : TcpServerSocket() {
    // No additional initialization needed
}

RtemsTcpServerSocket::~RtemsTcpServerSocket() {
    // No additional cleanup needed
}

SocketIpStatus RtemsTcpServerSocket::startup(SocketDescriptor& socketDescriptor) {
    PlatformIntType serverFd = -1;
    struct sockaddr_in address;
    
    // Create socket
    if ((serverFd = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        Fw::Logger::logMsg("RTEMS TcpServer: Failed to create socket: %d\n", errno);
        return SOCK_FAILED_TO_GET_SOCKET;
    }
    
    // Set socket options to allow reuse of address/port
    const int reuse = 1;
    if (::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        ::close(serverFd);
        Fw::Logger::logMsg("RTEMS TcpServer: Failed to set SO_REUSEADDR: %d\n", errno);
        return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
    }
    
    // Configure address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(this->m_port);
    
    // Convert IP address string to binary form
    if (IpSocket::addressToIp4(m_hostname, &(address.sin_addr)) != SOCK_SUCCESS) {
        ::close(serverFd);
        Fw::Logger::logMsg("RTEMS TcpServer: Invalid IP address: %s\n", m_hostname);
        return SOCK_INVALID_IP_ADDRESS;
    }
    
    // Bind socket to address
    if (::bind(serverFd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(serverFd);
        Fw::Logger::logMsg("RTEMS TcpServer: Failed to bind: %d\n", errno);
        return SOCK_FAILED_TO_BIND;
    }
    
    // Get the actual port number if port 0 was requested (dynamic allocation)
    socklen_t size = sizeof(address);
    if (::getsockname(serverFd, reinterpret_cast<struct sockaddr *>(&address), &size) == -1) {
        ::close(serverFd);
        Fw::Logger::logMsg("RTEMS TcpServer: Failed to get socket name: %d\n", errno);
        return SOCK_FAILED_TO_READ_BACK_PORT;
    }
    
    // Listen for incoming connections (backlog of 1 as per requirement)
    if (::listen(serverFd, 1) < 0) {
        ::close(serverFd);
        Fw::Logger::logMsg("RTEMS TcpServer: Failed to listen: %d\n", errno);
        return SOCK_FAILED_TO_LISTEN;
    }
    
    Fw::Logger::logMsg("RTEMS TcpServer: Listening on %s:%hu\n", m_hostname, ntohs(address.sin_port));
    
    // Store the server file descriptor
    socketDescriptor.serverFd = serverFd;
    
    // Update the port with the actual port number (important for port 0)
    this->m_port = ntohs(address.sin_port);
    
    return SOCK_SUCCESS;
}

SocketIpStatus RtemsTcpServerSocket::openProtocol(SocketDescriptor& socketDescriptor) {
    PlatformIntType clientFd = -1;
    PlatformIntType serverFd = socketDescriptor.serverFd;
    
    // Check if server socket is valid
    if (serverFd == -1) {
        Fw::Logger::logMsg("RTEMS TcpServer: Server socket not started\n");
        return SOCK_NOT_STARTED;
    }
    
    // Accept a connection
    clientFd = ::accept(serverFd, nullptr, nullptr);
    if (clientFd < 0) {
        Fw::Logger::logMsg("RTEMS TcpServer: Failed to accept: %d\n", errno);
        return SOCK_FAILED_TO_ACCEPT;
    }
    
    // Set up socket timeouts
    if (this->m_timeoutSeconds > 0 || this->m_timeoutMicroseconds > 0) {
        struct timeval timeout;
        timeout.tv_sec = static_cast<time_t>(this->m_timeoutSeconds);
        timeout.tv_usec = static_cast<suseconds_t>(this->m_timeoutMicroseconds);
        
        if (::setsockopt(clientFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            ::close(clientFd);
            Fw::Logger::logMsg("RTEMS TcpServer: Failed to set send timeout: %d\n", errno);
            return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
        }
    }
    
    // Get client info for logging
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    if (::getpeername(clientFd, (struct sockaddr*)&clientAddr, &addrLen) == 0) {
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);
        Fw::Logger::logMsg("RTEMS TcpServer: Accepted connection from %s:%hu\n", 
                         clientIp, ntohs(clientAddr.sin_port));
    }
    
    // Store the client file descriptor
    socketDescriptor.fd = clientFd;
    
    return SOCK_SUCCESS;
}

I32 RtemsTcpServerSocket::sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    // Ensure the socket is valid
    if (socketDescriptor.fd < 0) {
        errno = EBADF;
        return -1;
    }
    
    // Send data
    I32 bytesSent = ::send(socketDescriptor.fd, data, size, SOCKET_IP_SEND_FLAGS);
    
    if (bytesSent < 0) {
        Fw::Logger::logMsg("RTEMS TcpServer: Send error: %d\n", errno);
    }
    
    return bytesSent;
}

I32 RtemsTcpServerSocket::recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) {
    // Ensure the socket is valid
    if (socketDescriptor.fd < 0) {
        errno = EBADF;
        return -1;
    }
    
    // Receive data
    I32 bytesReceived = ::recv(socketDescriptor.fd, data, size, SOCKET_IP_RECV_FLAGS);
    
    if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        Fw::Logger::logMsg("RTEMS TcpServer: Receive error: %d\n", errno);
    }
    
    return bytesReceived;
}

} // namespace RTEMS
} // namespace Drv