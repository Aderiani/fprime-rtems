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

#ifdef __rtems__
#include <stdio.h>
#define OS_INIT_DEBUG(msg) do { printf("OS Init: %s\n", msg); fflush(stdout); } while(0)
#else
#define OS_INIT_DEBUG(msg) /* empty in non-RTEMS builds */
#endif
namespace Os {

    void init() {

        Os::Console::init();
        Os::FileSystem::init();
        Os::Cpu::init();
        Os::Memory::init();
        Os::Task::init();
    }

}  // namespace Os