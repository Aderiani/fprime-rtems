// ======================================================================
// \title  FatalHandlerComponentRTEMSImpl.cpp
// \author [your_name]
// \brief  RTEMS implementation of FatalHandler component for GR740
//
// ======================================================================

#include <Fw/Logger/Logger.hpp>
#include <Svc/FatalHandler/FatalHandlerComponentImpl.hpp>
#include <Os/Task.hpp>
#include <FpConfig.hpp>
#include <rtems.h>
#include <stdio.h>

namespace Svc {

    void FatalHandlerComponentImpl::FatalReceive_handler(
            const FwIndexType portNum,
            FwEventIdType Id) {
        
        // Step 1: Log the FATAL event immediately
        Fw::Logger::log("FATAL %" PRI_FwEventIdType " handled.\n", Id);
        
        // Step 2: Force flush any buffered logs
        fflush(stdout);
        fflush(stderr);
        
        // Step 3: Delay to allow the FATAL event to propagate to ground
        // This gives time for telemetry to be downlinked
        (void)Os::Task::delay(Fw::TimeInterval(1, 0));
        
        // Step 4: Log additional diagnostic information
        rtems_id current_task_id;
        rtems_status_code status = rtems_task_ident(RTEMS_SELF, 0, &current_task_id);
        if (status == RTEMS_SUCCESSFUL) {
            Fw::Logger::log("FATAL occurred in task ID: 0x%08x\n", current_task_id);
        }
        
        // Step 5: Decide on action based on configuration
        // Option A: Suspend the calling task (for debugging)
        #ifdef FATAL_HANDLER_DEBUG_MODE
            Fw::Logger::log("Suspending task due to FATAL. System continues for debugging.\n");
            fflush(stdout);
            rtems_task_suspend(RTEMS_SELF);
        #else
            // Option B: System reset (production behavior)
            Fw::Logger::log("Initiating system reset due to FATAL.\n");
            fflush(stdout);
            
            // Give a small delay for final log to be sent
            rtems_task_wake_after(rtems_clock_get_ticks_per_second() / 10); // 100ms
            
            // Perform system reset
            // Note: The exact reset method depends on your BSP
            // For GR740, you might use:
            #ifdef LEON3
                // LEON3/GR740 specific reset
                // This writes to the system register to cause a reset
                *((volatile unsigned int*) 0x80000004) = 0x0;
            #else
                // Generic RTEMS shutdown
                rtems_shutdown_executive(1);
            #endif
        #endif
        
        // Safety: Should never reach here, but just in case
        Fw::Logger::log("FATAL handler failed to halt system!\n");
        while (true) {
            rtems_task_wake_after(rtems_clock_get_ticks_per_second());
        }
    }

} // end namespace Svc