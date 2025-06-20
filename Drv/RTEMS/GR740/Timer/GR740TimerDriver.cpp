// GR740TimerDriver.cpp
#include "Drv/RTEMS/GR740/Timer/GR740TimerDriver.hpp"
#include "Fw/Types/Assert.hpp"
#include "Fw/Logger/Logger.hpp"

// Additional RTEMS includes for GPTIMER
extern "C" {
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp.h>
#include <grlib/grlib.h>
}

namespace Drv {

// Timer unit to use (avoid 0 and 1 since RTEMS may use them)
static const U32 GPTIMER_UNIT = 2;

// Base clock frequency for GR740 (50MHz system clock)
static const U32 GPTIMER_BASE_CLOCK_HZ = 50000000;

GR740TimerDriver::GR740TimerDriver(const char* const name) :
    GR740TimerDriverComponentBase(name),
    m_timerHz(0),
    m_initialized(false),
    m_running(false),
    m_cycleCount(0),
    m_interruptCount(0),
    m_ticksPerCycle(0),
    m_gptimerHandle(nullptr),
    m_timerUnit(GPTIMER_UNIT),
    m_timerPrescaler(0),
    m_timerReloadValue(0),
    m_hardwareInitialized(false)
{
    m_lastTickTime.now(); // Initialize last tick time
}

GR740TimerDriver::~GR740TimerDriver() {
    if (m_running) {
        stop();
    }
    cleanupGptimer();
}

bool GR740TimerDriver::initialize(U32 timerHz) {
    if (m_initialized) {
        return true; // Already initialized
    }
    
    m_timerHz = timerHz;
    
    // Calculate ticks per cycle for backup method
    m_ticksPerCycle = rtems_clock_get_ticks_per_second() / m_timerHz;
    if (m_ticksPerCycle < 1) {
        m_ticksPerCycle = 1; // Minimum of 1 tick
    }
    
    // Initialize GPTIMER hardware
    if (!initializeGptimerHardware()) {
        Fw::Logger::log("GR740TimerDriver: Failed to initialize GPTIMER hardware, falling back to software timing");
        // Continue with software-only mode
    }
    
    // Test connection to make sure the port is connected
    Os::RawTime testTime;
    testTime.now();
    this->CycleOut_out(0, testTime);

    m_initialized = true;
    this->log_ACTIVITY_HI_TimerInitialized(m_timerHz);
    this->tlmWrite_TimerFrequency(m_timerHz);
    
    return true;
}

bool GR740TimerDriver::initializeGptimerHardware() {
    // Initialize Timer Library
    if (tlib_init() != 0) {
        this->log_WARNING_HI_TimerError(0x1001); // TLIB init failed
        return false;
    }
    
    // Open timer unit
    m_gptimerHandle = tlib_open(m_timerUnit);
    if (m_gptimerHandle == nullptr) {
        this->log_WARNING_HI_TimerError(0x1002); // Timer open failed
        return false;
    }
    
    // Calculate prescaler and reload values
    // Formula: Timer_freq = Base_clock / ((prescaler + 1) * (reload + 1))
    // We want: Timer_freq = m_timerHz
    // So: (prescaler + 1) * (reload + 1) = Base_clock / m_timerHz
    
    U32 divider = GPTIMER_BASE_CLOCK_HZ / m_timerHz;
    
    // Find optimal prescaler and reload combination
    // Prefer higher prescaler to reduce interrupt frequency precision requirements
    m_timerPrescaler = 0;
    m_timerReloadValue = divider - 1;
    
    // If reload value is too large (>65535), increase prescaler
    while (m_timerReloadValue > 65535 && m_timerPrescaler < 255) {
        m_timerPrescaler++;
        m_timerReloadValue = (divider / (m_timerPrescaler + 1)) - 1;
    }
    
    if (m_timerReloadValue > 65535) {
        this->log_WARNING_HI_TimerError(0x1003); // Cannot achieve frequency
        tlib_close(m_gptimerHandle);
        m_gptimerHandle = nullptr;
        return false;
    }
    
    // Configure timer for periodic mode
    if (!configureGptimerPeriodic()) {
        tlib_close(m_gptimerHandle);
        m_gptimerHandle = nullptr;
        return false;
    }
    
    m_hardwareInitialized = true;
    this->log_ACTIVITY_HI_GptimerHwInitialized(m_timerUnit);
    
    return true;
}

bool GR740TimerDriver::configureGptimerPeriodic() {
    if (m_gptimerHandle == nullptr) {
        return false;
    }
    
    // Set prescaler
    if (tlib_set_prescaler(m_gptimerHandle, m_timerPrescaler) != 0) {
        this->log_WARNING_HI_TimerError(0x1004); // Prescaler set failed
        return false;
    }
    
    // Set reload value for periodic operation
    if (tlib_set_reload(m_gptimerHandle, m_timerReloadValue) != 0) {
        this->log_WARNING_HI_TimerError(0x1005); // Reload set failed
        return false;
    }
    
    // Register ISR
    if (tlib_irq_register(m_gptimerHandle, s_gptimerISR, this) != 0) {
        this->log_WARNING_HI_TimerError(0x1006); // ISR registration failed
        return false;
    }
    
    this->log_ACTIVITY_HI_IsrRegistered(m_timerUnit);
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
    
    m_running = true;
    m_lastTickTime.now(); // Reset last tick time
    
    if (m_hardwareInitialized && m_gptimerHandle != nullptr) {
        // Start hardware timer with interrupts enabled
        if (tlib_start(m_gptimerHandle, TLIB_FLAGS_BROADCAST | TLIB_FLAGS_IRQ_ENABLE) == 0) {
            this->log_ACTIVITY_HI_TimerStarted();
            Fw::Logger::log("GR740TimerDriver: Hardware timer started at %u Hz", m_timerHz);
            return true;
        } else {
            this->log_WARNING_HI_TimerError(0x1007); // Timer start failed
            Fw::Logger::log("GR740TimerDriver: Hardware timer start failed, continuing without ISR");
        }
    }
    
    // If hardware timer failed or not available, still mark as started for manual mode
    this->log_ACTIVITY_HI_TimerStarted();
    Fw::Logger::log("GR740TimerDriver: Timer started in software mode at %u Hz", m_timerHz);
    return true;
}

void GR740TimerDriver::stop() {
    if (!m_running) {
        return; // Not running
    }
    
    m_running = false;
    
    // Stop hardware timer if running
    if (m_hardwareInitialized && m_gptimerHandle != nullptr) {
        tlib_stop(m_gptimerHandle);
    }
    
    this->log_ACTIVITY_HI_TimerStopped();
}

void GR740TimerDriver::cleanupGptimer() {
    if (m_gptimerHandle != nullptr) {
        tlib_stop(m_gptimerHandle);
        tlib_irq_unregister(m_gptimerHandle);
        tlib_close(m_gptimerHandle);
        m_gptimerHandle = nullptr;
    }
    m_hardwareInitialized = false;
}

// Static ISR callback for GPTIMER
void GR740TimerDriver::s_gptimerISR(void* arg) {
    FW_ASSERT(arg);
    // Cast argument to component instance
    GR740TimerDriver* compPtr = static_cast<GR740TimerDriver*>(arg);
    
    // Post to component's message queue via internal port (async)
    // This allows ISR to return quickly
    compPtr->InterruptReport_internalInterfaceInvoke(0);
}

// Internal interrupt handler (runs in component thread context)
// This is called asynchronously from the component's message queue
void GR740TimerDriver::InterruptReport_internalInterfaceHandler(U32 interrupt) {
    if (!m_running) {
        return;
    }
    
    // Get current time
    Os::RawTime currentTime;
    currentTime.now();
    
    // Call output timing signal to drive rate groups
    // This now executes in the component's thread, not ISR context
    this->CycleOut_out(0, currentTime);
    
    // Update telemetry (safe to do in component thread)
    m_cycleCount++;
    m_interruptCount++;
    this->tlmWrite_TimerCycles(m_cycleCount);
    this->tlmWrite_TimerInterrupts(m_interruptCount);
    
    m_lastTickTime = currentTime;
}

// Legacy manual methods (kept for backup/testing)
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
    
    // Get time difference in RTEMS ticks
    rtems_interval current_ticks = rtems_clock_get_ticks_since_boot();
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
    
    this->CycleOut_out(0, currentTime);
    
    m_cycleCount++;
    this->tlmWrite_TimerCycles(m_cycleCount);
    
    m_lastTickTime = currentTime;
}

} // namespace Drv