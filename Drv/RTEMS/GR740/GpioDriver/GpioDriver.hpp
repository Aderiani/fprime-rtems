// File: Drv/RTEMS/GR740/GpioDriver/GpioDriver.hpp
#ifndef DRV_GR740_GPIO_DRIVER_HPP
#define DRV_GR740_GPIO_DRIVER_HPP

#include <Drv/RTEMS/GR740/GpioDriver/GR740GpioDriverComponentAc.hpp>
#include <Drv/RTEMS/include/DriverCommon.hpp>

// Include our custom GPIO register definitions
#include <Drv/RTEMS/GR740/include/gr740_gpio.h>

namespace Drv {

  class GR740GpioDriver final : public GR740GpioDriverComponentBase {
  public:
    static constexpr FwSizeType MAX_GPIO_PINS = 32; // GRGPIO supports up to 32 pins

    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    GR740GpioDriver(
        const char* const compName /*!< The component name*/
    );

    ~GR740GpioDriver();

    // Use the enum from the FPP interface
    enum LocalDirection {
      DIR_IN = GpioDirection::GPIO_IN,
      DIR_OUT = GpioDirection::GPIO_OUT
    };

    enum InterruptTrigger {
      INT_TRIGGER_NONE,    //!< No interrupt
      INT_TRIGGER_RISING,  //!< Rising edge trigger
      INT_TRIGGER_FALLING, //!< Falling edge trigger
      INT_TRIGGER_BOTH     //!< Both rising and falling edge trigger
    };

    //! \brief Initialize the GPIO driver
    //!
    bool initialize(NATIVE_INT_TYPE instance = 0);

    //! \brief Configure a GPIO pin
    //!
    bool configurePin(
      NATIVE_UINT_TYPE pin,
      GpioDirection direction,
      Fw::Logic initialValue = Fw::Logic::LOW,
      InterruptTrigger interruptTrigger = INT_TRIGGER_NONE
    );

  PRIVATE:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for gpioRead
    //!
    Drv::GpioStatus gpioRead_handler(
        const FwIndexType portNum, /*!< The port number*/
        Fw::Logic& state/*!< The returned value*/
    ) override;

    //! Handler implementation for gpioWrite
    //!
    Drv::GpioStatus gpioWrite_handler(
        const FwIndexType portNum, /*!< The port number*/
        const Fw::Logic& state/*!< The value to be written*/
    ) override;

    //! \brief ISR callback function for GPIO interrupts
    //!
    static void isrCallback(void* arg);

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------
    struct drvmgr_dev* m_gpioDevice; //!< GRGPIO device pointer
    gr740_gpio_device m_gpioRegs;    //!< GRGPIO register structure
    bool m_initialized;              //!< Initialization flag
    Fw::Logic m_outputState[MAX_GPIO_PINS]; //!< Current output state of each pin
    GpioDirection m_pinDirection[MAX_GPIO_PINS]; //!< Direction configuration for each pin
  };

} // end namespace Drv

#endif