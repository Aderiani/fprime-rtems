// GR740WatchdogDriver.cpp
#include <Drv/RTEMS/GR740/Watchdog/GR740WatchdogDriver.hpp>
#include <Fw/Logger/Logger.hpp>

extern "C" {
#include <bsp.h>
#include <bsp/watchdog.h>
#include <rtems.h>  // For RTEMS functions
#include <leon.h>  // For watchdog functions
}

namespace Drv {

GR740WatchdogDriver::GR740WatchdogDriver(const char* const name)
    : GR740WatchdogDriverComponentBase(name), m_initialized(false) {}

GR740WatchdogDriver::~GR740WatchdogDriver() {}

bool GR740WatchdogDriver::initialize(U32 timeoutMs) {
    // The watchdog is already configured by bootloader/BSP
    // We just need to service it properly
    m_initialized = true;
    this->log_ACTIVITY_HI_WatchdogInitialized(timeoutMs);
    return true;
}

void GR740WatchdogDriver::StrokeWatchdog_handler(const FwIndexType portNum, U32 context) {
    if (!m_initialized) {
        return;
    }

    static U32 counter = 0;
    bsp_watchdog_reload(0, 0x1fffff); 

    if (++counter % 100 == 0) {
        this->log_DIAGNOSTIC_WatchdogStroked();
    }
}

}  // namespace Drv