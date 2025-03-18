#include <Drv/RTEMS/GR740/GpioDriver/GpioDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>

namespace Drv {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  GR740GpioDriver::GR740GpioDriver(const char* const compName) :
      GR740GpioDriverComponentBase(compName),
      m_gpioDevice(nullptr),
      m_gpioRegs(nullptr),
      m_initialized(false)
  {
    // Initialize pin state arrays
    for (NATIVE_UINT_TYPE i = 0; i < MAX_GPIO_PINS; i++) {
      m_outputState[i] = Fw::Logic::LOW;
      m_pinDirection[i] = Drv::GR740GpioDriver::GpioDirection::GPIO_INPUT;
    }
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

    // Set the register pointer using base address from device tree
    uintptr_t base_addr = (uintptr_t)m_gpioDevice->base_addr;
    
    // Set the register pointer
    m_gpioRegs = reinterpret_cast<volatile gr740_gpio_regs*>(base_addr);

    // Initialize the GPIO controller (reset it)
    m_gpioRegs->output = 0;
    m_gpioRegs->dir = 0;
    m_gpioRegs->imask = 0;

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_GpioInitSuccess();
    return true;
  }

  bool GR740GpioDriver::configurePin(
      NATIVE_UINT_TYPE pin,
      Drv::GR740GpioDriver::GpioDirection direction,
      Fw::Logic initialValue,
      bool enableInterrupt
  )
  {
    if (!m_initialized || m_gpioRegs == nullptr) {
      this->log_WARNING_HI_GpioInitError(pin, -1);
      return false;
    }

    // Verify pin number is valid
    if (pin >= MAX_GPIO_PINS) {
      this->log_WARNING_HI_GpioInitError(pin, -2);
      return false;
    }

    // Configure pin direction
    if (direction == Drv::GR740GpioDriver::GpioDirection::GPIO_OUTPUT) {
      // Set as GPIO_OUTPUT
      m_gpioRegs->dir |= (1U << pin);
      
      // Set initial value
      if (initialValue == Fw::Logic::HIGH) {
        m_gpioRegs->output |= (1U << pin);
        m_outputState[pin] = Fw::Logic::HIGH;
      } else {
        m_gpioRegs->output &= ~(1U << pin);
        m_outputState[pin] = Fw::Logic::LOW;
      }
    } else {
      // Set as input
      m_gpioRegs->dir &= ~(1U << pin);
      
      // Configure interrupts if requested
      if (enableInterrupt) {
        // Enable interrupt for this pin
        m_gpioRegs->imask |= (1U << pin);
      }
    }

    // Store pin direction
    m_pinDirection[pin] = direction;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(
      pin,
      (direction == Drv::GR740GpioDriver::GpioDirection::GPIO_OUTPUT) ? "OUTPUT" : "INPUT"
    );
    return true;
  }

 GpioStatus GR740GpioDriver::gpioRead_handler(
      const FwIndexType portNum,
      Fw::Logic& state
  )
  {
    if (!m_initialized || m_gpioRegs == nullptr) {
      this->log_WARNING_HI_GpioPinReadError(portNum);
      return GpioStatus::UNKNOWN_ERROR;
    }

    // Check if the pin is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
      this->log_WARNING_HI_GpioPinReadError(portNum);
      return GpioStatus::INVALID_MODE;
    }

    // Read pin value
    U32 pinValue = (m_gpioRegs->data >> portNum) & 0x1;
    state = (pinValue != 0) ? Fw::Logic::HIGH : Fw::Logic::LOW;

    this->log_DIAGNOSTIC_GpioPinReadSuccess(portNum);
    return GpioStatus::OP_OK;
  }

  GpioStatus GR740GpioDriver::gpioWrite_handler(
      const FwIndexType portNum,
      const Fw::Logic& state
  )
  {
    if (!m_initialized || m_gpioRegs == nullptr) {
      this->log_WARNING_HI_GpioInitError(portNum);
      return GpioStatus::NOT_OPENED;
    }

    // Check if the pin is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
      this->log_WARNING_HI_GpioInitError(portNum);
      return GpioStatus::INVALID_MODE;
    }

    // Check if the pin is configured as output
    if (m_pinDirection[portNum] != Drv::GR740GpioDriver::GpioDirection::GPIO_OUTPUT) {
      this->log_WARNING_HI_GpioInitError(portNum);
      return GpioStatus::INVALID_MODE;
    }

    // Write to the pin
    if (state == Fw::Logic::HIGH) {
      m_gpioRegs->output |= (1U << portNum);
    } else {
      m_gpioRegs->output &= ~(1U << portNum);
    }

    // Store current state
    m_outputState[portNum] = state;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(portNum);
    return GpioStatus::OP_OK;
  }

  void GR740GpioDriver::isrCallback(void* arg)
  {
    // This is a placeholder for GPIO interrupt handling
    GR740GpioDriver* driver = static_cast<GR740GpioDriver*>(arg);
    FW_ASSERT(driver != nullptr);
    
    // TODO: Implement specific interrupt handling logic
  }

} // end namespace Drv