// ======================================================================
// \title  RtemsUdpAdapter.hpp
// \author fprime-community
// \brief  RTEMS specific implementation for Drv::UdpSocket
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef RTEMS_UDP_ADAPTER_HPP
#define RTEMS_UDP_ADAPTER_HPP

#include <Drv/Ip/UdpSocket.hpp>
#include <IpCfg.hpp>

namespace Drv {
namespace RTEMS {

/**
 * \brief RTEMS-specific implementation of the UdpSocket
 * 
 * This class provides a thin adapter between the generic UdpSocket
 * interface and the RTEMS networking stack. The implementation is largely
 * identical to the Posix/Linux implementation since RTEMS uses a BSD
 * sockets API.
 */
class RtemsUdpSocket : public UdpSocket {
  public:
    /**
     * \brief Constructor
     */
    RtemsUdpSocket();

    /**
     * \brief Destructor
     */
    virtual ~RtemsUdpSocket();

  protected:
    /**
     * \brief RTEMS-specific implementation for binding a UDP socket
     * 
     * @param fd Socket file descriptor
     * @return Status of the operation
     */
    SocketIpStatus bind(const PlatformIntType fd) override;

    /**
     * \brief RTEMS-specific implementation for opening a UDP socket
     * 
     * @param socketDescriptor Descriptor to fill with UDP socket 
     * @return Status of the operation
     */
    SocketIpStatus openProtocol(SocketDescriptor& socketDescriptor) override;

    /**
     * \brief RTEMS-specific implementation for sending data
     * 
     * @param socketDescriptor Socket to send on
     * @param data Data buffer to send
     * @param size Size of data to send
     * @return Number of bytes sent or -1 for error
     */
    I32 sendProtocol(const SocketDescriptor& socketDescriptor, const U8* const data, const U32 size) override;

    /**
     * \brief RTEMS-specific implementation for receiving data
     * 
     * @param socketDescriptor Socket to receive from
     * @param data Buffer to store received data
     * @param size Size of data buffer
     * @return Number of bytes received or -1 for error
     */
    I32 recvProtocol(const SocketDescriptor& socketDescriptor, U8* const data, const U32 size) override;
};

} // namespace RTEMS
} // namespace Drv

#endif // RTEMS_UDP_ADAPTER_HPP