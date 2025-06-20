// GR740TimerDriver.hpp
#ifndef DRV_GR740_TIMER_DRIVER_HPP
#define DRV_GR740_TIMER_DRIVER_HPP

#include "Drv/RTEMS/GR740/Timer/GR740TimerDriverComponentAc.hpp"
#include <rtems.h>
#include <Os/RawTime.hpp>

// RTEMS Timer Library (TLIB) includes for GPTIMER
extern "C" {
#include <tlib.h>
#include <bsp.h>
}

namespace Drv {

class GR740TimerDriver : public GR740TimerDriverComponentBase {
  public:
    // Constructor
    GR740TimerDriver(const char* const name);
    
    // Destructor
    virtual ~GR740TimerDriver();
    
    // Initialize the timer with GPTIMER hardware
    bool initialize(U32 timerHz);
    
    // Start the hardware timer
    bool start();
    
    // Stop the hardware timer
    void stop();
    
    // Manual tick generation (backup method)
    void manualTick();
    
    // Check if it's time for a tick (backup method)
    bool checkTick(); 
    void generateTick();

  PRIVATE:
    // ISR handler for internal interrupt port
    void InterruptReport_internalInterfaceHandler(U32 interrupt);
    
    // Static ISR callback function for GPTIMER
    static void s_gptimerISR(void* arg);
    
    // Initialize GPTIMER hardware
    bool initializeGptimerHardware();
    
    // Configure GPTIMER for periodic interrupts
    bool configureGptimerPeriodic();
    
    // Clean up GPTIMER resources
    void cleanupGptimer();
    
    // Member variables
    U32 m_timerHz;                   // Timer frequency in Hz
    bool m_initialized;              // Initialization flag
    volatile bool m_running;         // Running state flag
    U32 m_cycleCount;                // Count of timer cycles for telemetry
    U32 m_interruptCount;            // Count of interrupts received
    rtems_interval m_ticksPerCycle;  // RTEMS ticks per cycle (backup)
    Os::RawTime m_lastTickTime;      // Time of last tick
    
    // GPTIMER hardware specifics
    void* m_gptimerHandle;           // TLIB timer handle
    U32 m_timerUnit;                 // GPTIMER unit number (1-6, avoiding 0 used by RTEMS)
    U32 m_timerPrescaler;            // Timer prescaler value
    U32 m_timerReloadValue;          // Timer reload value for frequency
    bool m_hardwareInitialized;     // Hardware initialization status
};

} // namespace Drv

#endif // DRV_GR740_TIMER_DRIVER_HPP