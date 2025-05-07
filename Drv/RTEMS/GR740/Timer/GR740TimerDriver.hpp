// GR740TimerDriver.hpp
#ifndef DRV_GR740_TIMER_DRIVER_HPP
#define DRV_GR740_TIMER_DRIVER_HPP

#include "Drv/RTEMS/GR740/Timer/GR740TimerDriverComponentAc.hpp"
#include <rtems.h>
#include <Os/RawTime.hpp>

namespace Drv {

class GR740TimerDriver : public GR740TimerDriverComponentBase {
  public:
    // Constructor
    GR740TimerDriver(const char* const name);
    
    // Destructor
    virtual ~GR740TimerDriver();
    
    // Initialize the timer
    bool initialize(U32 timerHz);
    
    // Start the timer
    bool start();
    
    // Stop the timer
    void stop();
    
    // Manually trigger a tick - to be called from the main loop
    void manualTick();
    
    // Check if it's time for a tick
    bool checkTick(); 
    void generateTick();

    
  PRIVATE:
    U32 m_timerHz;                   // Timer frequency in Hz
    bool m_initialized;              // Initialization flag
    volatile bool m_running;         // Running state flag
    U32 m_cycleCount;                // Count of timer cycles for telemetry
    rtems_interval m_ticksPerCycle;  // RTEMS ticks per cycle
    Os::RawTime m_lastTickTime;      // Time of last tick
};

} // namespace Drv

#endif // DRV_GR740_TIMER_DRIVER_HPP