// File: Drv/RTEMS/GR740/GpioDriver/GpioDriver.cpp
#include <Drv/RTEMS/GR740/GpioDriver/GpioDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>

namespace Drv {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  GR740GpioDriver::GR740GpioDriver(const char* const compName) :
      GR740GpioDriverComponentBase(compName),
      m_gpioDevice(nullptr),
      m_initialized(false)
  {
    // Initialize pin state arrays
    for (NATIVE_UINT_TYPE i = 0; i < MAX_GPIO_PINS; i++) {
      m_outputState[i] = Fw::Logic::LOW;
      m_pinDirection[i] = GpioDirection::GPIO_IN;
    }
    
    // Initialize register pointer to null
    m_gpioRegs.regs = nullptr;
  }

  GR740GpioDriver::~GR740GpioDriver()
  {
    // No cleanup needed as the driver manager handles device resources
  }

  bool GR740GpioDriver::initialize(NATIVE_INT_TYPE instance)
  {
    if (m_initialized) {
      return true; // Already initialized
    }

    // Initialize driver manager if not already initialized by BSP
    RTEMS::DriverUtil::initializeDriverManager();

    // Find the GPIO device
    m_gpioDevice = RTEMS::DriverUtil::findAmbaDevice(
      RTEMS::GAISLER_VENDOR_ID,
      RTEMS::DeviceId::GRGPIO,
      instance
    );

    if (m_gpioDevice == nullptr) {
      this->log_WARNING_HI_GpioInitError();
      return false;
    }

    // Get the base address for the registers
    // This is a simplified approach - you may need to adjust based on your BSP
    uintptr_t base_addr = (uintptr_t)m_gpioDevice->drv_priv;
    
    // Set the register pointer
    m_gpioRegs.regs = reinterpret_cast<volatile gr740_gpio_regs*>(base_addr);

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_GpioInitSuccess();
    return true;
  }

  bool GR740GpioDriver::configurePin(
      NATIVE_UINT_TYPE pin,
      GpioDirection direction,
      Fw::Logic initialValue,
      InterruptTrigger interruptTrigger
  )
  {
    if (!m_initialized || m_gpioRegs.regs == nullptr) {
      return false;
    }

    // Verify pin number is valid
    if (pin >= MAX_GPIO_PINS) {
      return false;
    }

    // Configure pin direction
    if (direction == GpioDirection::GPIO_OUT) {
      // Set as output
      m_gpioRegs.regs->dir |= (1U << pin);
      
      // Set initial value
      if (initialValue == Fw::Logic::HIGH) {
        m_gpioRegs.regs->data |= (1U << pin);
        m_outputState[pin] = Fw::Logic::HIGH;
      } else {
        m_gpioRegs.regs->data &= ~(1U << pin);
        m_outputState[pin] = Fw::Logic::LOW;
      }
    } else {
      // Set as input
      m_gpioRegs.regs->dir &= ~(1U << pin);
    }

    // Store pin direction
    m_pinDirection[pin] = direction;

    // Register for interrupts if needed
    if (direction == GpioDirection::GPIO_IN && interruptTrigger != INT_TRIGGER_NONE) {
      // Clear any pending interrupts for this pin
      m_gpioRegs.regs->ipol = (1U << pin);

      // Set interrupt trigger type
      if (interruptTrigger == INT_TRIGGER_RISING) {
        m_gpioRegs.regs->edge |= (1U << pin);  // Edge triggered
        m_gpioRegs.regs->ipol &= ~(1U << pin); // Rising edge
      } else if (interruptTrigger == INT_TRIGGER_FALLING) {
        m_gpioRegs.regs->edge |= (1U << pin);  // Edge triggered
        m_gpioRegs.regs->ipol |= (1U << pin);  // Falling edge
      }

      // Enable the interrupt
      m_gpioRegs.regs->imask |= (1U << pin);
      
      this->log_DIAGNOSTIC_GpioInterruptRegistered(pin);
    }

    return true;
  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  Drv::GpioStatus GR740GpioDriver::gpioRead_handler(
      const FwIndexType portNum,
      Fw::Logic& state
  )
  {
    if (!m_initialized || m_gpioRegs.regs == nullptr) {
      return GpioStatus::GPIO_INVALID_MODE;
    }

    // Check if the pin is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
      return GpioStatus::GPIO_INVALID_PIN;
    }

    // Read pin value
    U32 pinValue = (m_gpioRegs.regs->data >> portNum) & 0x1;
    state = (pinValue != 0) ? Fw::Logic::HIGH : Fw::Logic::LOW;

    this->log_DIAGNOSTIC_GpioPinReadSuccess(portNum);
    return GpioStatus::GPIO_OK;
  }

  Drv::GpioStatus GR740GpioDriver::gpioWrite_handler(
      const FwIndexType portNum,
      const Fw::Logic& state
  )
  {
    if (!m_initialized || m_gpioRegs.regs == nullptr) {
      return GpioStatus::GPIO_INVALID_MODE;
    }

    // Check if the pin is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
      return GpioStatus::GPIO_INVALID_PIN;
    }

    // Check if the pin is configured as output
    if (m_pinDirection[portNum] != GpioDirection::GPIO_OUT) {
      this->log_WARNING_HI_GpioPinWriteError(portNum);
      return GpioStatus::GPIO_INVALID_DIRECTION;
    }

    // Write to the pin
    if (state == Fw::Logic::HIGH) {
      m_gpioRegs.regs->data |= (1U << portNum);
    } else {
      m_gpioRegs.regs->data &= ~(1U << portNum);
    }

    // Store current state
    m_outputState[portNum] = state;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(portNum);
    return GpioStatus::GPIO_OK;
  }

  void GR740GpioDriver::isrCallback(void* arg)
  {
    // Get driver instance
    GR740GpioDriver* driver = static_cast<GR740GpioDriver*>(arg);
    FW_ASSERT(driver != nullptr);
    
    if (!driver->m_initialized || driver->m_gpioRegs.regs == nullptr) {
      return;
    }

    // Get timestamp
    Os::RawTime timestamp;
    timestamp.now();

    // Send interrupt notification on all connected ports
    // In a real implementation, you'd determine which pin triggered 
    // the interrupt and use the appropriate port
    driver->gpioInterrupt_out(0, timestamp);
  }

} // end namespace Drv