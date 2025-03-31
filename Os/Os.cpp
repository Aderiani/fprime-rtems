// ======================================================================
// \title Os/Os.cpp
// \brief common definitions for the OSAL layer
// ======================================================================
#include "Os/Os.hpp"
#include "FpConfig.h"
#include "Os/Console.hpp"
#include "Os/Cpu.hpp"
#include "Os/FileSystem.hpp"
#include "Os/Memory.hpp"
#include "Os/Task.hpp"

// For debugging during initialization
#ifdef __rtems__
#include <stdio.h>
#define OS_INIT_DEBUG(msg) printf("OS Init: %s\n", msg)
#else
#define OS_INIT_DEBUG(msg) /* empty in non-RTEMS builds */
#endif
namespace Os {

void init() {

    Os::Console::init();
    OS_INIT_DEBUG("Console initialized");
    Os::FileSystem::init();
    Os::Cpu::init();
    Os::Memory::init();
    Os::Task::init();
}

}  // namespace Os