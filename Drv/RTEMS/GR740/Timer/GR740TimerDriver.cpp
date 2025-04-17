#include "Drv/RTEMS/GR740/Timer/GR740TimerDriver.hpp"
#include "Fw/Types/Assert.hpp"
#include "Fw/Logger/Logger.hpp"
#include <rtems.h>

#include <Fw/Types/Assert.hpp>
#include <Os/RawTime.hpp>

namespace Drv {

GR740TimerDriver::GR740TimerDriver(const char* const name) :
    GR740TimerDriverComponentBase(name),
    m_timerHz(0),
    m_timerId(0),
    m_initialized(false),
    m_running(false),
    m_cycleCount(0)
{
}

GR740TimerDriver::~GR740TimerDriver() {
    // Stop the timer if it's running
    if (m_running) {
        stop();
    }
}

bool GR740TimerDriver::initialize(U32 timerHz) {
    if (m_initialized) {
        return true; // Already initialized
    }

    m_timerHz = timerHz;
    Fw::Logger::log("GR740TimerDriver: Initializing timer with frequency %u Hz", timerHz);

    // Create a timer
    rtems_status_code status = rtems_timer_create(
        rtems_build_name('T', 'M', 'R', '1'),
        &m_timerId
    );

    if (status != RTEMS_SUCCESSFUL) {
        Fw::Logger::log("GR740TimerDriver: Failed to create timer, status = %d", status);
        return false;
    }

    m_initialized = true;
    this->log_ACTIVITY_HI_TimerInitialized(m_timerHz);
    return true;
}

bool GR740TimerDriver::start() {
    if (!m_initialized) {
        Fw::Logger::log("GR740TimerDriver: Cannot start timer - not initialized");
        return false;
    }

    if (m_running) {
        return true; // Already running
    }

    rtems_status_code status;
    rtems_interval ticks = rtems_clock_get_ticks_per_second() / m_timerHz;

    if (ticks < 1) {
        ticks = 1; // Minimum of 1 tick
    }

    Fw::Logger::log("GR740TimerDriver: Starting timer with %u ticks per interval", 
                   static_cast<unsigned int>(ticks));

    // Start the timer, which will call our timerISR function
    // Note the proper function signature for RTEMS timer service routine
    status = rtems_timer_fire_after(
        m_timerId,
        ticks, 
        GR740TimerDriver::timerISR,
        reinterpret_cast<void*>(this)
    );

    if (status != RTEMS_SUCCESSFUL) {
        Fw::Logger::log("GR740TimerDriver: Failed to start timer, status = %d", status);
        return false;
    }

    m_running = true;
    this->log_ACTIVITY_HI_TimerStarted();
    return true;
}

void GR740TimerDriver::stop() {
    if (!m_running) {
        return; // Not running
    }

    rtems_status_code status = rtems_timer_cancel(m_timerId);
    if (status != RTEMS_SUCCESSFUL) {
        Fw::Logger::log("GR740TimerDriver: Failed to stop timer, status = %d", status);
    }

    m_running = false;
    this->log_ACTIVITY_HI_TimerStopped();
}

// This is the RTEMS timer service routine with the correct signature
void GR740TimerDriver::timerISR(rtems_id timer_id, void* arg) {
    // Get the component instance from the argument
    GR740TimerDriver* timerDriver = reinterpret_cast<GR740TimerDriver*>(arg);
    
    // Call the handler method
    timerDriver->handleTimerTick();
    
    // Restart the timer for the next cycle (periodic operation)
    if (timerDriver->m_running) {
        rtems_interval ticks = rtems_clock_get_ticks_per_second() / timerDriver->m_timerHz;
        if (ticks < 1) ticks = 1;
        
        rtems_timer_fire_after(
            timer_id,
            ticks,
            GR740TimerDriver::timerISR,
            arg
        );
    }
}

void GR740TimerDriver::handleTimerTick() {
    // Get current time using Os::RawTime as required by port
    Os::RawTime rawTime;
    rawTime.now();
    
    // Call output timing signal to drive rate groups
    this->CycleOut_out(0, rawTime);
    
    // Increment cycle count and emit telemetry
    m_cycleCount++;
    this->tlmWrite_TimerCycles(m_cycleCount);
}

} // namespace Drv