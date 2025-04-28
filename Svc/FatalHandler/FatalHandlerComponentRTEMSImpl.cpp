// ======================================================================
// \title  FatalHandlerComponentRTEMSImpl.cpp
// \author your_name
// \brief  RTEMS implementation of FatalHandler component
//
// ======================================================================

#include <Fw/Logger/Logger.hpp>
#include <Svc/FatalHandler/FatalHandlerComponentImpl.hpp>
#include <rtems.h>

namespace Svc {

    void FatalHandlerComponentImpl::FatalReceive_handler(
            const FwIndexType portNum,
            FwEventIdType Id) {
        // Log the FATAL but don't terminate
        Fw::Logger::log("FATAL %d received, but continuing\n", Id);
                
        // Don't exit or suspend - just return
    }

} // end namespace Svc