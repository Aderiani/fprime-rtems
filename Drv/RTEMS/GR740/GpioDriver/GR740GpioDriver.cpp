#include <Drv/RTEMS/GR740/GpioDriver/GR740GpioDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>

namespace Drv {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

GR740GpioDriver::GR740GpioDriver(const char* const compName)
    : GR740GpioDriverComponentBase(compName), m_gpioRegs(nullptr), m_initialized(false) {
    // Initialize pin state arrays
    for (NATIVE_UINT_TYPE i = 0; i < MAX_GPIO_PINS; i++) {
        m_outputState[i] = Fw::Logic::LOW;
        m_pinDirection[i] = GpioDirection::GPIO_DIRECTION_INPUT;
    }
}

GR740GpioDriver::~GR740GpioDriver() {
    // No cleanup needed
}

// GR740GpioDriver.cpp

bool GR740GpioDriver::initialize() {
    if (m_initialized) {
        return true;  // Already initialized
    }

    // GPIO pin configuration registers according to bdinit.c
    volatile uint32_t* ftmen_reg = reinterpret_cast<volatile uint32_t*>(0xFFA0B000);
    volatile uint32_t* alten_reg = reinterpret_cast<volatile uint32_t*>(0xFFA0B004);

    // Disable alternate function for GPIO2[8:0] so they can be used as GPIOs
    // Note: GPIO2[8:5] control LEDs on GR740-MINI
    *alten_reg = 0x3FFFFF & ~0x1FF;

    // Disable FTMCTRL function for appropriate pins
    *ftmen_reg = 0x3FFFFF & ~((1 << 21) | (1 << 9) | 0x1FF);

    // Set the register pointer to the known base address for GPIO
    m_gpioRegs = reinterpret_cast<volatile gr740_gpio_regs*>(RTEMS::BaseAddress::GPIO);

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_GpioInitSuccess();
    return true;
}

GpioStatus GR740GpioDriver::gpioWrite_handler(const FwIndexType portNum, const Fw::Logic& state) {
    if (!m_initialized || m_gpioRegs == nullptr) {
        this->log_WARNING_HI_GpioPinWriteError(portNum);
        return GpioStatus::NOT_OPENED;
    }

    // Check if the pin is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
        this->log_WARNING_HI_GpioPinWriteError(portNum);
        return GpioStatus::INVALID_MODE;
    }

    // Check if the pin is configured as output
    if (m_pinDirection[portNum] != GpioDirection::GPIO_DIRECTION_OUTPUT) {
        this->log_WARNING_HI_GpioPinWriteError(portNum);
        return GpioStatus::INVALID_MODE;
    }

    // On GR740, LEDs are on GPIO2[8:5], so adjust the pin number if needed
    // This assumes portNum 0-3 should map to LEDs on GPIO2[5-8]
    U32 actualPin = portNum;
    if (portNum < 4) {
        // Map to LED pins (GPIO2[5-8])
        actualPin = portNum + 5;
    }

    // Write to the pin
    if (state == Fw::Logic::HIGH) {
        m_gpioRegs->output |= (1U << actualPin);
    } else {
        m_gpioRegs->output &= ~(1U << actualPin);
    }

    // Store current state
    m_outputState[portNum] = state;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(portNum);
    return GpioStatus::OP_OK;
}

bool GR740GpioDriver::configurePin(NATIVE_UINT_TYPE pin, GpioDirection direction, Fw::Logic initialValue) {
    if (!m_initialized || m_gpioRegs == nullptr) {
        this->log_WARNING_HI_GpioInitError();
        return false;
    }

    // Verify pin number is valid
    if (pin >= MAX_GPIO_PINS) {
        return false;
    }

    // Configure pin direction
    if (direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
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

        // Configure interrupts if needed based on direction
        if (direction == GpioDirection::GPIO_DIRECTION_INTERRUPT_RISING ||
            direction == GpioDirection::GPIO_DIRECTION_INTERRUPT_BOTH) {
            // Enable rising edge detection
            m_gpioRegs->edge |= (1U << pin);
        }

        if (direction == GpioDirection::GPIO_DIRECTION_INTERRUPT_FALLING ||
            direction == GpioDirection::GPIO_DIRECTION_INTERRUPT_BOTH) {
            // Enable falling edge detection
            m_gpioRegs->ipol |= (1U << pin);
        }

        if (direction != GpioDirection::GPIO_DIRECTION_INPUT) {
            // Enable interrupt for the pin
            m_gpioRegs->imask |= (1U << pin);
        }
    }

    // Store pin direction
    m_pinDirection[pin] = direction;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(pin);
    return true;
}

GpioStatus GR740GpioDriver::gpioRead_handler(const FwIndexType portNum, Fw::Logic& state) {
    if (!m_initialized || m_gpioRegs == nullptr) {
        this->log_WARNING_HI_GpioPinReadError(portNum);
        return GpioStatus::NOT_OPENED;
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


}  // end namespace Drv