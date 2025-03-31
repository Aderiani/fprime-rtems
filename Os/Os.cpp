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
        // Absolute minimal initialization
        printf("OS Init: Starting minimal initialization\n");
        fflush(stdout);
        
        // Only initialize Console for now
        printf("OS Init: Initializing Console\n");
        fflush(stdout);
        Os::Console::init();
        printf("OS Init: Console initialized\n");
        fflush(stdout);
        
        printf("OS Init: Minimal initialization complete\n");
        fflush(stdout);
    }

}  // namespace Os