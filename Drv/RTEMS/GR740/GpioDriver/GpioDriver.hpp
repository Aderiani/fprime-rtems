#ifndef DRV_GR740_GPIO_DRIVER_HPP
#define DRV_GR740_GPIO_DRIVER_HPP

#include <Drv/RTEMS/GR740/GpioDriver/GR740GpioDriverComponentAc.hpp>
#include <Drv/RTEMS/include/DriverCommon.hpp>

// Include our custom GPIO register definitions
#include <Drv/RTEMS/GR740/include/gr740_gpio.h>

// RTEMS and standard headers
#include <rtems.h>
#include <drvmgr/drvmgr.h>

namespace Drv {

  class GR740GpioDriver final : public GR740GpioDriverComponentBase {
  public:
    static constexpr FwSizeType MAX_GPIO_PINS = 32; // GRGPIO supports up to 32 pins

    // Constructor and Destructor
    GR740GpioDriver(const char* const compName);
    ~GR740GpioDriver();

    enum GpioDirection {
      GPIO_OUTPUT,                                   //!< Output GPIO pin for direct writing
      GPIO_INPUT,                                    //!< Input GPIO pin for direct reading
      GPIO_INTERRUPT_RISING_EDGE,                    //!< Input GPIO pin triggers interrupt port on rising edge
      GPIO_INTERRUPT_FALLING_EDGE,                   //!< Input GPIO pin triggers interrupt port on falling edge
      GPIO_INTERRUPT_BOTH_RISING_AND_FALLING_EDGES,  //!< Input GPIO pin triggers interrupt port on both edges
      MAX_GPIO_CONFIGURATION
  };

    //! \brief Initialize the GPIO driver
    bool initialize(NATIVE_INT_TYPE instance = 0);

    //! \brief Configure a GPIO pin
    bool configurePin(
      NATIVE_UINT_TYPE pin,
      Drv::GR740GpioDriver::GpioDirection direction,
      Fw::Logic initialValue = Fw::Logic::LOW,
      bool enableInterrupt = false
    );

  PRIVATE:
    // Handler implementations for user-defined typed input ports
   GpioStatus gpioRead_handler(
        const FwIndexType portNum,
        Fw::Logic& state
    ) override;

   GpioStatus gpioWrite_handler(
        const FwIndexType portNum,
        const Fw::Logic& state
    ) override;

    //! \brief ISR callback function for GPIO interrupts
    static void isrCallback(void* arg);

    // Member variables
    struct drvmgr_dev* m_gpioDevice; //!< GRGPIO device pointer
    volatile gr740_gpio_regs* m_gpioRegs; //!< GRGPIO register structure
    bool m_initialized;              //!< Initialization flag
    Fw::Logic m_outputState[MAX_GPIO_PINS]; //!< Current output state of each pin
    GpioDirection m_pinDirection[MAX_GPIO_PINS]; //!< Direction configuration for each pin
  };

} // end namespace Drv

#endif // DRV_GR740_GPIO_DRIVER_HPP