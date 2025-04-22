// GR740WatchdogDriver.hpp
#ifndef DRV_GR740_WATCHDOG_DRIVER_HPP
#define DRV_GR740_WATCHDOG_DRIVER_HPP

#include <Drv/RTEMS/GR740/Watchdog/GR740WatchdogDriverComponentAc.hpp>

namespace Drv {

class GR740WatchdogDriver : public GR740WatchdogDriverComponentBase {
  public:
    GR740WatchdogDriver(const char* const name);
    ~GR740WatchdogDriver();

    // Initialize the watchdog
    bool initialize(U32 timeoutMs);

  PRIVATE:
    // Handler implementations
    void StrokeWatchdog_handler(const FwIndexType portNum, U32 context) override;

    // Hardware register addresses
    static constexpr U32 GPTIMER0_BASE = 0xff908000;
    static constexpr U32 TIMER0_CONTROL = GPTIMER0_BASE + 0x18;
    static constexpr U32 TIMER0_RELOAD = GPTIMER0_BASE + 0x14;

    bool m_initialized;
};

} // namespace Drv

#endif