// ======================================================================
// \title  GpioDriver.cpp
// \author fprime-community
// \brief  cpp file for GR740GpioDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/RTEMS/GR740/GpioDriver/GpioDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>

namespace Drv {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  GR740GpioDriver ::
    GR740GpioDriver(const char* const compName) :
      GR740GpioDriverComponentBase(compName),
      m_gpioDevice(nullptr),
      m_initialized(false)
  {
    // Initialize pin state arrays
    for (NATIVE_UINT_TYPE i = 0; i < MAX_GPIO_PINS; i++) {
      m_outputState[i] = Fw::Logic::LOW;
      m_pinDirection[i] = GPIO_DIRECTION_INPUT;
    }
  }

  GR740GpioDriver ::
    ~GR740GpioDriver()
  {
    // No cleanup needed as the driver manager handles device resources
  }

  bool GR740GpioDriver ::
    initialize(NATIVE_INT_TYPE instance)
  {
    if (m_initialized) {
      return true; // Already initialized
    }

    // Initialize driver manager if not already initialized by BSP
    RTEMS::DriverUtil::initializeDriverManager();

    // Find the GRGPIO device in the system
    m_gpioDevice = RTEMS::DriverUtil::findAmbaDevice(
      RTEMS::GAISLER_VENDOR_ID,
      RTEMS::DeviceId::GRGPIO,
      instance
    );

    if (m_gpioDevice == nullptr) {
      this->log_WARNING_HI_GpioInitError();
      return false;
    }

    // Get the GRGPIO register structure
    struct ambapp_dev* ambapp_dev = (struct ambapp_dev*)m_gpioDevice->businfo;
    m_gpioRegs.regs = (volatile struct grgpio_regs*)ambapp_dev->apb_slv->start;

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_GpioInitSuccess();
    return true;
  }

  bool GR740GpioDriver ::
    configurePin(
      NATIVE_UINT_TYPE pin,
      GpioDirection direction,
      Fw::Logic initialValue,
      GpioInterruptTrigger interruptTrigger
    )
  {
    if (!m_initialized) {
      return false;
    }

    // Verify pin number is valid
    if (pin >= MAX_GPIO_PINS) {
      return false;
    }

    // Configure pin direction
    if (direction == GPIO_DIRECTION_OUTPUT) {
      // Set as output
      m_gpioRegs.regs->dir |= (1 << pin);
      
      // Set initial value
      if (initialValue == Fw::Logic::HIGH) {
        m_gpioRegs.regs->data |= (1 << pin);
        m_outputState[pin] = Fw::Logic::HIGH;
      } else {
        m_gpioRegs.regs->data &= ~(1 << pin);
        m_outputState[pin] = Fw::Logic::LOW;
      }
    } else {
      // Set as input
      m_gpioRegs.regs->dir &= ~(1 << pin);
    }

    // Store pin direction
    m_pinDirection[pin] = direction;

    // Configure interrupts if this is an input pin
    if (direction == GPIO_DIRECTION_INPUT && interruptTrigger != GPIO_INT_TRIGGER_NONE) {
      // Clear any pending interrupts for this pin
      m_gpioRegs.regs->iclear = (1 << pin);

      // Configure interrupt polarity
      switch (interruptTrigger) {
        case GPIO_INT_TRIGGER_RISING:
          m_gpioRegs.regs->edge |= (1 << pin);  // Edge triggered
          m_gpioRegs.regs->level &= ~(1 << pin); // Rising edge
          break;
          
        case GPIO_INT_TRIGGER_FALLING:
          m_gpioRegs.regs->edge |= (1 << pin);   // Edge triggered
          m_gpioRegs.regs->level |= (1 << pin);  // Falling edge
          break;
          
        case GPIO_INT_TRIGGER_BOTH:
          // Not directly supported by GRGPIO, would need to toggle settings
          return false;
          
        default:
          break;
      }

      // Register ISR with driver manager
      int status = drvmgr_interrupt_register(
        m_gpioDevice,
        pin,  // IRQ number
        "GR740GpioDriver",
        GR740GpioDriver::isrCallback,
        this
      );

      if (status != 0) {
        this->log_WARNING_HI_GpioInterruptError(pin);
        return false;
      }

      // Enable interrupt for this pin
      m_gpioRegs.regs->imask |= (1 << pin);
      this->log_DIAGNOSTIC_GpioInterruptRegistered(pin);
    }

    return true;
  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  GpioStatus GR740GpioDriver ::
    gpioRead_handler(
        const FwIndexType portNum,
        Fw::Logic& state
    )
  {
    if (!m_initialized) {
      return GpioStatus::GPIO_INVALID_MODE;
    }

    // Check if the pin is in the valid range
    if (portNum >= MAX_GPIO_PINS) {
      return GpioStatus::GPIO_INVALID_PIN;
    }

    // Read pin value
    U32 pinValue = (m_gpioRegs.regs->data >> portNum) & 0x1;
    state = (pinValue != 0) ? Fw::Logic::HIGH : Fw::Logic::LOW;

    this->log_DIAGNOSTIC_GpioPinReadSuccess(portNum);
    return GpioStatus::GPIO_OK;
  }

  GpioStatus GR740GpioDriver ::
    gpioWrite_handler(
        const FwIndexType portNum,
        const Fw::Logic& state
    )
  {
    if (!m_initialized) {
      return GpioStatus::GPIO_INVALID_MODE;
    }

    // Check if the pin is in the valid range
    if (portNum >= MAX_GPIO_PINS) {
      return GpioStatus::GPIO_INVALID_PIN;
    }

    // Check if the pin is configured as output
    if (m_pinDirection[portNum] != GPIO_DIRECTION_OUTPUT) {
      this->log_WARNING_HI_GpioPinWriteError(portNum);
      return GpioStatus::GPIO_INVALID_MODE;
    }

    // Write to the pin
    if (state == Fw::Logic::HIGH) {
      m_gpioRegs.regs->data |= (1 << portNum);
    } else {
      m_gpioRegs.regs->data &= ~(1 << portNum);
    }

    // Store current state
    m_outputState[portNum] = state;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(portNum);
    return GpioStatus::GPIO_OK;
  }

  void GR740GpioDriver ::
    isrCallback(void* arg)
  {
    // Get driver instance
    GR740GpioDriver* driver = static_cast<GR740GpioDriver*>(arg);
    FW_ASSERT(driver != nullptr);

    // Check which pins have pending interrupts
    U32 pendingInts = driver->m_gpioRegs.regs->ipend;

    // Handle each triggered pin
    for (NATIVE_UINT_TYPE pin = 0; pin < MAX_GPIO_PINS; pin++) {
      if (pendingInts & (1 << pin)) {
        // Clear the interrupt
        driver->m_gpioRegs.regs->iclear = (1 << pin);

        // Get the current timestamp
        Os::RawTime timestamp;
        timestamp.now();

        // Send interrupt notification
        driver->gpioInterrupt_out(0, timestamp);
      }
    }
  }

} // end namespace Drv