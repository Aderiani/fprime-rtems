// GR740TimerDriver.cpp
#include "Drv/RTEMS/GR740/Timer/GR740TimerDriver.hpp"
#include "Fw/Types/Assert.hpp"
#include "Fw/Logger/Logger.hpp"

namespace Drv {

GR740TimerDriver::GR740TimerDriver(const char* const name) :
    GR740TimerDriverComponentBase(name),
    m_timerHz(0),
    m_initialized(false),
    m_running(false),
    m_cycleCount(0),
    m_ticksPerCycle(0)
{
    m_lastTickTime.now(); // Initialize last tick time
}

GR740TimerDriver::~GR740TimerDriver() {
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
    
    // Calculate ticks per cycle
    m_ticksPerCycle = rtems_clock_get_ticks_per_second() / m_timerHz;
    if (m_ticksPerCycle < 1) {
        m_ticksPerCycle = 1; // Minimum of 1 tick
    }
    
        // A one-time test call to make sure the port is connected
        Os::RawTime testTime;
        testTime.now();
        // printf("GR740TimerDriver: Testing CycleOut port connection\n");
        this->CycleOut_out(0, testTime);


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
    
    // printf("GR740TimerDriver: Starting timer with %u ticks per interval\n", 
        //    static_cast<unsigned int>(m_ticksPerCycle));
    
    // Set running flag
    m_running = true;
    m_lastTickTime.now(); // Reset last tick time
    
    this->log_ACTIVITY_HI_TimerStarted();
    return true;
}

void GR740TimerDriver::stop() {
    if (!m_running) {
        return; // Not running
    }
    
    m_running = false;
    this->log_ACTIVITY_HI_TimerStopped();
}

void GR740TimerDriver::manualTick() {
    if (!m_running) {
        return;
    }
    
    Os::RawTime currentTime;
    currentTime.now();
    
    this->CycleOut_out(0, currentTime);
    
    m_cycleCount++;
    this->tlmWrite_TimerCycles(static_cast<U32>(m_cycleCount));
    
    m_lastTickTime = currentTime;
}

bool GR740TimerDriver::checkTick() {
    if (!m_running || !m_initialized) {
        return false;
    }
    
    // Os::RawTime currentTime;
    // currentTime.now();
    
    // Get time difference in RTEMS ticks by using the rtems_clock functions directly
    rtems_interval current_ticks = rtems_clock_get_ticks_since_boot();
    
    // Convert the m_ticksPerCycle to a simple elapsed time check
    // Store the last tick time in ticks for easier comparison
    static rtems_interval lastTickInTicks = current_ticks;
    
    if (current_ticks - lastTickInTicks >= m_ticksPerCycle) {
        lastTickInTicks = current_ticks;
        return true;
    }
    
    return false;
}

void GR740TimerDriver::generateTick() {
    if (!m_running) {
        return;
    }
    
    Os::RawTime currentTime;
    currentTime.now();
    
    // printf("[TIMER] Manually generating timer tick #%u\n", m_cycleCount);
    this->CycleOut_out(0, currentTime);
    
    m_cycleCount++;
    this->tlmWrite_TimerCycles(m_cycleCount);
    
    m_lastTickTime = currentTime;
}

} // namespace Drv