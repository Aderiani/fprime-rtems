// ======================================================================
// \title  DefaultTcpServerAdapter.cpp
// \author fprime-community
// \brief  Sets default TcpServerSocket to RTEMS implementation via linker
//
// \copyright
// Copyright (C) 2023 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/Ip/TcpServerSocket.hpp>
#include "RtemsTcpServerAdapter.hpp"

namespace Drv {
    // Override the default TcpServerSocket with the RTEMS implementation
    TcpServerSocket* createTcpServerSocket() {
        return new RTEMS::RtemsTcpServerSocket();
    }
}