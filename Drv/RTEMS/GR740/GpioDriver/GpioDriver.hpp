#ifndef DRV_GR740_GPIO_DRIVER_HPP
#define DRV_GR740_GPIO_DRIVER_HPP

// Include the component-specific generated header
#include <Drv/RTEMS/GR740/GpioDriver/GR740GpioDriverComponentAc.hpp>

// Include our driver common definitions
#include <Drv/RTEMS/include/DriverCommon.hpp>

// Include our custom GPIO register definitions (C header)
#ifdef __cplusplus
extern "C" {
#endif
#include <Drv/RTEMS/GR740/include/gr740_gpio.h>
#ifdef __cplusplus
}
#endif

namespace Drv {

  class GR740GpioDriver final : public GR740GpioDriverComponentBase {
  public:
    static constexpr FwSizeType MAX_GPIO_PINS = 32; // GRGPIO supports up to 32 pins

    // Constructor and Destructor
    GR740GpioDriver(const char* const compName);
    ~GR740GpioDriver();

    enum GpioDirection {
      GPIO_DIRECTION_OUTPUT = 0,               //!< Output GPIO pin for direct writing
      GPIO_DIRECTION_INPUT = 1,                //!< Input GPIO pin for direct reading
      GPIO_DIRECTION_INTERRUPT_RISING = 2,     //!< Input GPIO pin triggers interrupt port on rising edge
      GPIO_DIRECTION_INTERRUPT_FALLING = 3,    //!< Input GPIO pin triggers interrupt port on falling edge
      GPIO_DIRECTION_INTERRUPT_BOTH = 4,       //!< Input GPIO pin triggers interrupt port on both edges
      GPIO_DIRECTION_MAX
    };

    //! \brief Initialize the GPIO driver
    bool initialize();

    //! \brief Configure a GPIO pin
    bool configurePin(
      NATIVE_UINT_TYPE pin,
      GpioDirection direction,
      Fw::Logic initialValue = Fw::Logic::LOW
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

    // Member variables
    volatile gr740_gpio_regs* m_gpioRegs; //!< GRGPIO register structure
    bool m_initialized;              //!< Initialization flag
    Fw::Logic m_outputState[MAX_GPIO_PINS]; //!< Current output state of each pin
    GpioDirection m_pinDirection[MAX_GPIO_PINS]; //!< Direction configuration for each pin
  };

} // end namespace Drv

#endif // DRV_GR740_GPIO_DRIVER_HPP