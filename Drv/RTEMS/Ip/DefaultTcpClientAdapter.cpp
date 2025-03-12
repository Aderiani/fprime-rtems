// ======================================================================
// \title  DefaultUdpAdapter.cpp
// \author fprime-community
// \brief  Sets default UdpSocket to RTEMS implementation via linker
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/Ip/UdpSocket.hpp>
#include "RtemsUdpAdapter.hpp"

namespace Drv {
    // Override the default UdpSocket with the RTEMS implementation
    UdpSocket* createUdpSocket() {
        return new RTEMS::RtemsUdpSocket();
    }
}
