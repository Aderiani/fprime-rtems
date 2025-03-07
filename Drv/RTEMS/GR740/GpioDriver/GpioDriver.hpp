// ======================================================================
// \title  GpioDriver.hpp
// \author fprime-community
// \brief  hpp file for GR740GpioDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_GR740_GPIO_DRIVER_HPP
#define DRV_GR740_GPIO_DRIVER_HPP

#include <Drv/RTEMS/GR740/GpioDriver/GpioDriverComponentAc.hpp>
#include <Drv/RTEMS/include/DriverCommon.hpp>

// GRLIB GRGPIO driver includes
#include <grlib/grgpio.h>

namespace Drv {

  class GR740GpioDriver final : public GR740GpioDriverComponentBase {
  public:
    static constexpr FwSizeType MAX_GPIO_PINS = 32; // GRGPIO supports up to 32 pins

    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object GR740GpioDriver
    //!
    GR740GpioDriver(
        const char* const compName /*!< The component name*/
    );

    //! Destroy object GR740GpioDriver
    //!
    ~GR740GpioDriver();

    // ----------------------------------------------------------------------
    // GPIO configuration types
    // ----------------------------------------------------------------------
    enum GpioDirection {
      GPIO_DIRECTION_INPUT,  //!< Configure pin as input
      GPIO_DIRECTION_OUTPUT  //!< Configure pin as output
    };

    enum GpioInterruptTrigger {
      GPIO_INT_TRIGGER_NONE,    //!< No interrupt
      GPIO_INT_TRIGGER_RISING,  //!< Rising edge trigger
      GPIO_INT_TRIGGER_FALLING, //!< Falling edge trigger
      GPIO_INT_TRIGGER_BOTH     //!< Both rising and falling edge trigger
    };

    //! \brief Initialize the GPIO driver
    //!
    //! This function initializes the GPIO driver by finding the GRGPIO device
    //! in the system and setting up the driver.
    //!
    //! \param instance The instance number of the GRGPIO device (default: 0)
    //! \return true if initialization was successful, false otherwise
    bool initialize(NATIVE_INT_TYPE instance = 0);

    //! \brief Configure a GPIO pin
    //!
    //! \param pin GPIO pin number
    //! \param direction Direction (input or output)
    //! \param initialValue Initial value for output pins
    //! \param interruptTrigger Interrupt trigger type (if input)
    //! \return true if configuration was successful, false otherwise
    bool configurePin(
      NATIVE_UINT_TYPE pin,
      GpioDirection direction,
      Fw::Logic initialValue = Fw::Logic::LOW,
      GpioInterruptTrigger interruptTrigger = GPIO_INT_TRIGGER_NONE
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
    //! \param arg Pointer to this instance
    static void isrCallback(void* arg);

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------
    struct drvmgr_dev* m_gpioDevice; //!< GRGPIO device pointer
    GRGPIO_Device m_gpioRegs;       //!< GRGPIO register structure
    bool m_initialized;              //!< Initialization flag
    Fw::Logic m_outputState[MAX_GPIO_PINS]; //!< Current output state of each pin
    GpioDirection m_pinDirection[MAX_GPIO_PINS]; //!< Direction configuration for each pin
  };

} // end namespace Drv

#endif