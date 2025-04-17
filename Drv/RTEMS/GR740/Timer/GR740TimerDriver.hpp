#ifndef DRV_GR740_TIMER_DRIVER_HPP
#define DRV_GR740_TIMER_DRIVER_HPP

#include <Fw/Logger/Logger.hpp>
#include <rtems.h>
#include "Drv/RTEMS/GR740/Timer/GR740TimerDriverComponentAc.hpp"

namespace Drv {

/**
 * @brief Driver for GR740 GPTIMER to drive F' Rate Groups
 *
 * This component uses the RTEMS GPTIMER driver to generate
 * interrupts at a specified rate and drive F' rate groups.
 */
class GR740TimerDriver : public GR740TimerDriverComponentBase {
  public:
    /**
     * @brief Construct a new GR740TimerDriver
     * 
     * @param name Component name
     */
    GR740TimerDriver(const char* const name);

    /**
     * @brief Destroy the GR740TimerDriver
     */
    virtual ~GR740TimerDriver();

    /**
     * @brief Initialize the timer driver
     * 
     * @param timerHz Timer frequency in Hz
     * @return true if initialization successful
     * @return false if initialization failed
     */
    bool initialize(U32 timerHz);

    /**
     * @brief Start the timer
     * 
     * @return true if start successful
     * @return false if start failed
     */
    bool start();

    /**
     * @brief Stop the timer
     */
    void stop();

  PRIVATE:
    //! RTEMS timer service routine with proper signature
    static void timerISR(rtems_id timer_id, void* arg);

    //! Method to handle the actual timer callback work
    void handleTimerTick();

    //! Timer frequency in Hz
    U32 m_timerHz;

    //! RTEMS timer ID
    rtems_id m_timerId;

    //! Flag indicating if timer is initialized
    bool m_initialized;

    //! Flag indicating if timer is running
    bool m_running;

    //! Cycle count
    U32 m_cycleCount;
};

} // namespace Drv

#endif // DRV_GR740_TIMER_DRIVER_HPP