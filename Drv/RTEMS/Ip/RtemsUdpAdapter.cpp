// ======================================================================
// \title  RtemsUdpAdapter.cpp
// \author fprime-community
// \brief  RTEMS specific implementation for Drv::UdpSocket
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include "RtemsUdpAdapter.hpp"
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

namespace Drv {
namespace RTEMS {

RtemsUdpSocket::RtemsUdpSocket() : UdpSocket() {
    // No additional initialization needed
}

RtemsUdpSocket::~RtemsUdpSocket() {
    // No additional cleanup needed
}

SocketIpStatus RtemsUdpSocket::bind(const PlatformIntType fd) {
    struct sockaddr_in address;
    FW_ASSERT(fd != -1);

    // Set up the address port and name
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(this->m_recv_port);

    // First IP address to socket sin_addr
    if (IpSocket::addressToIp4(m_recv_hostname, &(address.sin_addr)) != SOCK_SUCCESS) {
        Fw::Logger::logMsg("RTEMS UDP: Invalid IP address: %s\n", m_recv_hostname);
        return SOCK_INVALID_IP_ADDRESS;
    }

    // UDP (for receiving) requires bind to an address to the socket
    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        Fw::Logger::logMsg("RTEMS UDP: Failed to bind: %d\n", errno);
        return SOCK_FAILED_TO_BIND;
    }

    // Get the actual port number if port 0 was requested (dynamic allocation)
    socklen_t size = sizeof(address);
    if (::getsockname(fd, reinterpret_cast<struct sockaddr *>(&address), &size) == -1) {
        Fw::Logger::logMsg("RTEMS UDP: Failed to get socket name: %d\n", errno);
        return SOCK_FAILED_TO_READ_BACK_PORT;
    }

    FW_ASSERT(sizeof(this->m_state->m_addr_recv) == sizeof(address), static_cast<FwAssertArgType>(sizeof(this->m_state->m_addr_recv)), static_cast<FwAssertArgType>(sizeof(address)));
    memcpy(&this->m_state->m_addr_recv, &address, sizeof(this->m_state->m_addr_recv));

    // Store the actual port number for port 0 (dynamic allocation)
    this->m_recv_port = ntohs(address.sin_port);

    return SOCK_SUCCESS;
}

SocketIpStatus RtemsUdpSocket::openProtocol(SocketDescriptor& socketDescriptor) {
    SocketIpStatus status = SOCK_SUCCESS;
    NATIVE_INT_TYPE socketFd = -1;
    struct sockaddr_in address;

    U16 port = this->m_port;

    // Acquire a socket, or return error
    if ((socketFd = ::socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        Fw::Logger::logMsg("RTEMS UDP: Failed to create socket: %d\n", errno);
        return SOCK_FAILED_TO_GET_SOCKET;
    }

    // May not be sending in all cases
    if (port != 0) {
        // Set up the address port and name
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(this->m_port);

        // First IP address to socket sin_addr
        if ((status = IpSocket::addressToIp4(m_hostname, &(address.sin_addr))) != SOCK_SUCCESS) {
            ::close(socketFd);
            Fw::Logger::logMsg("RTEMS UDP: Invalid IP address: %s\n", m_hostname);
            return status;
        }

        // Now apply timeouts
        if ((status = IpSocket::setupTimeouts(socketFd)) != SOCK_SUCCESS) {
            ::close(socketFd);
            Fw::Logger::logMsg("RTEMS UDP: Failed to set timeouts: %d\n", errno);
            return status;
        }
        FW_ASSERT(sizeof(this->m_state->m_addr_send) == sizeof(address), static_cast<FwAssertArgType>(sizeof(this->m_state->m_addr_send)), static_cast<FwAssertArgType>(sizeof(address)));
        memcpy(&this->m_state->m_addr_send, &address, sizeof(this->m_state->m_addr_send));
    }

    // Receive port set up only done when configure receive was called
    U16 recv_port = this->m_recv_port;
    if (recv_port != 0) {
        status = this->bind(socketFd);
        // When we are setting up for receiving as well, then we must bind to a port
        if (status != SOCK_SUCCESS) {
            (void) ::close(socketFd); // Closing FD as a retry will reopen send side
            Fw::Logger::logMsg("RTEMS UDP: Failed to bind for receiving: %d\n", errno);
            return status;
        }
    }

    // Log message for UDP
    if ((port == 0) && (recv_port > 0)) {
        Fw::Logger::logMsg("RTEMS UDP: Setup to only receive UDP at %s:%hu\n", m_recv_hostname, recv_port);
    } else if ((port > 0) && (recv_port == 0))  {
        Fw::Logger::logMsg("RTEMS UDP: Setup to only send UDP at %s:%hu\n", m_hostname, port);
    } else if ((port > 0) && (recv_port > 0))  {
        Fw::Logger::logMsg("RTEMS UDP: Setup to receive UDP at %s:%hu and send to %s:%hu\n", 
                          m_recv_hostname, 
                          recv_port,
                          m_hostname,
                          port);
    }
    // Neither configuration method was called - this is an error
    else {
        FW_ASSERT(port > 0 || recv_port > 0, static_cast<FwAssertArgType>(port), static_cast<FwAssertArgType>(recv_port));
    }
    
    FW_ASSERT(status == SOCK_SUCCESS, static_cast<FwAssertArgType>(status));
    socketDescriptor.fd = socketFd;
    return status;
}

I32 RtemsUdpSocket::sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) {
    FW_ASSERT(this->m_state->m_addr_send.sin_family != 0); // Make sure the address was previously setup
    
    I32 sent = static_cast<I32>(::sendto(socketDescriptor.fd, 
                                          data, 
                                          size, 
                                          SOCKET_IP_SEND_FLAGS,
                                          reinterpret_cast<struct sockaddr *>(&this->m_state->m_addr_send), 
                                          sizeof(this->m_state->m_addr_send)));
    
    if (sent < 0) {
        Fw::Logger::logMsg("RTEMS UDP: Send error: %d\n", errno);
    }
    
    return sent;
}

I32 RtemsUdpSocket::recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) {
    FW_ASSERT(this->m_state->m_addr_recv.sin_family != 0); // Make sure the address was previously setup
    
    I32 received = static_cast<I32>(::recvfrom(socketDescriptor.fd, 
                                                data, 
                                                size, 
                                                SOCKET_IP_RECV_FLAGS, 
                                                nullptr, 
                                                nullptr));
    
    if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        Fw::Logger::logMsg("RTEMS UDP: Receive error: %d\n", errno);
    }
    
    return received;
}

} // namespace RTEMS
} // namespace Drv