// ======================================================================
// \title  RtemsTcpClientAdapter.cpp
// \author fprime-community
// \brief  RTEMS specific implementation for Drv::TcpClientSocket
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include "RtemsTcpClientAdapter.hpp"
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

RtemsTcpClientSocket::RtemsTcpClientSocket() : TcpClientSocket() {
    // No additional initialization needed
}

RtemsTcpClientSocket::~RtemsTcpClientSocket() {
    // No additional cleanup needed
}

SocketIpStatus RtemsTcpClientSocket::openProtocol(SocketDescriptor& socketDescriptor) {
    PlatformIntType socketFd = -1;
    struct sockaddr_in address;
    
    // Create socket
    if ((socketFd = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        Fw::Logger::logMsg("RTEMS TcpClient: Failed to create socket: %d\n", errno);
        return SOCK_FAILED_TO_GET_SOCKET;
    }
    
    // Configure address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(this->m_port);
    
    // Convert IP address string to binary form
    if (IpSocket::addressToIp4(m_hostname, &(address.sin_addr)) != SOCK_SUCCESS) {
        ::close(socketFd);
        Fw::Logger::logMsg("RTEMS TcpClient: Invalid IP address: %s\n", m_hostname);
        return SOCK_INVALID_IP_ADDRESS;
    }
    
    // Set up socket timeouts
    if (this->m_timeoutSeconds > 0 || this->m_timeoutMicroseconds > 0) {
        struct timeval timeout;
        timeout.tv_sec = static_cast<time_t>(this->m_timeoutSeconds);
        timeout.tv_usec = static_cast<suseconds_t>(this->m_timeoutMicroseconds);
        
        if (::setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            ::close(socketFd);
            Fw::Logger::logMsg("RTEMS TcpClient: Failed to set send timeout: %d\n", errno);
            return SOCK_FAILED_TO_SET_SOCKET_OPTIONS;
        }
    }
    
    // Connect to server
    if (::connect(socketFd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(socketFd);
        Fw::Logger::logMsg("RTEMS TcpClient: Failed to connect: %d\n", errno);
        return SOCK_FAILED_TO_CONNECT;
    }
    
    // Log successful connection
    char serverIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), serverIp, INET_ADDRSTRLEN);
    Fw::Logger::logMsg("RTEMS TcpClient: Connected to %s:%hu\n", 
                      serverIp, ntohs(address.sin_port));
    
    // Store the file descriptor
    socketDescriptor.fd = socketFd;
    
    return SOCK_SUCCESS;
}

I32 RtemsTcpClientSocket::sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    // Ensure the socket is valid
    if (socketDescriptor.fd < 0) {
        errno = EBADF;
        return -1;
    }
    
    // Send data
    I32 bytesSent = ::send(socketDescriptor.fd, data, size, SOCKET_IP_SEND_FLAGS);
    
    if (bytesSent < 0) {
        Fw::Logger::logMsg("RTEMS TcpClient: Send error: %d\n", errno);
    }
    
    return bytesSent;
}

I32 RtemsTcpClientSocket::recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) {
    // Ensure the socket is valid
    if (socketDescriptor.fd < 0) {
        errno = EBADF;
        return -1;
    }
    
    // Receive data
    I32 bytesReceived = ::recv(socketDescriptor.fd, data, size, SOCKET_IP_RECV_FLAGS);
    
    if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        Fw::Logger::logMsg("RTEMS TcpClient: Receive error: %d\n", errno);
    }
    
    return bytesReceived;
}

} // namespace RTEMS
} // namespace Drv