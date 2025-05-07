#include <Drv/RTEMS/GR740/GpioDriver/GR740GpioDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/Assert.hpp>

namespace Drv {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

GR740GpioDriver::GR740GpioDriver(const char* const compName)
    : GR740GpioDriverComponentBase(compName), 
      m_initialized(false),
      m_gpio0_data_reg(nullptr),
      m_gpio0_dir_reg(nullptr),
      m_gpio1_data_reg(nullptr),
      m_gpio1_dir_reg(nullptr) {
    // Initialize pin state arrays
    for (NATIVE_UINT_TYPE i = 0; i < MAX_GPIO_PINS; i++) {
        m_outputState[i] = Fw::Logic::LOW;
        m_pinDirection[i] = GpioDirection::GPIO_DIRECTION_INPUT;
    }
}

GR740GpioDriver::~GR740GpioDriver() {
    // No cleanup needed
}

bool GR740GpioDriver::initialize() {
    if (m_initialized) {
        return true;  // Already initialized
    }

    // Direct volatile pointer approach for GPIO register access
    m_gpio0_data_reg = reinterpret_cast<volatile uint32_t*>(GPIO0_BASE_ADDR + GPIO_DATA_OUT_OFFSET);
    m_gpio0_dir_reg = reinterpret_cast<volatile uint32_t*>(GPIO0_BASE_ADDR + GPIO_DIRECTION_OFFSET);
    m_gpio1_data_reg = reinterpret_cast<volatile uint32_t*>(GPIO1_BASE_ADDR + GPIO_DATA_OUT_OFFSET);
    m_gpio1_dir_reg = reinterpret_cast<volatile uint32_t*>(GPIO1_BASE_ADDR + GPIO_DIRECTION_OFFSET);

    // Read and log current register values for debugging
    Fw::Logger::log("GPIO0 direction register initial value: 0x%08x\n", *m_gpio0_dir_reg);
    Fw::Logger::log("GPIO1 direction register initial value: 0x%08x\n", *m_gpio1_dir_reg);
    Fw::Logger::log("GPIO0 data register initial value: 0x%08x\n", *m_gpio0_data_reg);
    Fw::Logger::log("GPIO1 data register initial value: 0x%08x\n", *m_gpio1_data_reg);

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_GpioInitSuccess();
    return true;
}

GpioStatus GR740GpioDriver::gpioWrite_handler(const FwIndexType portNum, const Fw::Logic& state) {
    if (!m_initialized) {
        this->initialize();
        if (!m_initialized) {
            this->log_WARNING_HI_GpioPinWriteError(portNum);
            return GpioStatus::NOT_OPENED;
        }
    }

    // Check if the port is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
        this->log_WARNING_HI_GpioPinWriteError(portNum);
        return GpioStatus::INVALID_MODE;
    }

    // Determine which GPIO controller and bit to use based on portNum
    volatile uint32_t* data_reg = nullptr;
    volatile uint32_t* dir_reg = nullptr;
    uint32_t pin_mask = 0;

    // Map portNum to specific GPIO pin
    // LED7 is GPIO2[5], LED8 is GPIO2[6]
    switch (portNum) {
        case 0: // LED7 (GPIO2[5])
            data_reg = m_gpio1_data_reg;  // Second controller (GPIO1)
            dir_reg = m_gpio1_dir_reg;
            pin_mask = LED7_BIT;
            break;
        case 1: // LED8 (GPIO2[6])
            data_reg = m_gpio1_data_reg;  // Second controller (GPIO1)
            dir_reg = m_gpio1_dir_reg;
            pin_mask = LED8_BIT;
            break;
        default:
            // Try to use original pin mapping for other pins
            if (portNum < 4) {
                // For pins 0-3, use GPIO1 controller
                data_reg = m_gpio1_data_reg;
                dir_reg = m_gpio1_dir_reg;
                pin_mask = (1U << portNum);
            } else {
                // For pins 4+, use GPIO0 controller
                data_reg = m_gpio0_data_reg;
                dir_reg = m_gpio0_dir_reg;
                pin_mask = (1U << (portNum - 4));
            }
            break;
    }

    // Ensure pin is configured as output
    *dir_reg |= pin_mask;

    // Set or clear the appropriate bit based on state
    if (state == Fw::Logic::HIGH) {
        *data_reg |= pin_mask;  // Set bit
    } else {
        *data_reg &= ~pin_mask; // Clear bit
    }

    // Store current state
    m_outputState[portNum] = state;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(portNum);
    return GpioStatus::OP_OK;
}

bool GR740GpioDriver::configurePin(NATIVE_UINT_TYPE pin, GpioDirection direction, Fw::Logic initialValue) {
    if (!m_initialized) {
        this->initialize();
        if (!m_initialized) {
            this->log_WARNING_HI_GpioInitError();
            return false;
        }
    }

    // Verify pin number is valid
    if (pin >= MAX_GPIO_PINS) {
        return false;
    }

    // Determine which controller to use based on pin number
    volatile uint32_t* data_reg = nullptr;
    volatile uint32_t* dir_reg = nullptr;
    uint32_t pin_mask = 0;

    // Map pin to specific GPIO
    switch (pin) {
        case 0: // LED7 (GPIO2[5])
            data_reg = m_gpio1_data_reg;
            dir_reg = m_gpio1_dir_reg;
            pin_mask = LED7_BIT;
            break;
        case 1: // LED8 (GPIO2[6])
            data_reg = m_gpio1_data_reg;
            dir_reg = m_gpio1_dir_reg;
            pin_mask = LED8_BIT;
            break;
        default:
            // Try to use original pin mapping for other pins
            if (pin < 4) {
                // For pins 0-3, use GPIO1 controller
                data_reg = m_gpio1_data_reg;
                dir_reg = m_gpio1_dir_reg;
                pin_mask = (1U << pin);
            } else {
                // For pins 4+, use GPIO0 controller
                data_reg = m_gpio0_data_reg;
                dir_reg = m_gpio0_dir_reg;
                pin_mask = (1U << (pin - 4));
            }
            break;
    }

    // Configure pin direction
    if (direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
        // Set as GPIO_OUTPUT
        *dir_reg |= pin_mask;

        // Set initial value
        if (initialValue == Fw::Logic::HIGH) {
            *data_reg |= pin_mask;
            m_outputState[pin] = Fw::Logic::HIGH;
        } else {
            *data_reg &= ~pin_mask;
            m_outputState[pin] = Fw::Logic::LOW;
        }
    } else {
        // Set as input - clear direction bit
        *dir_reg &= ~pin_mask;
    }

    // Store pin direction
    m_pinDirection[pin] = direction;

    this->log_DIAGNOSTIC_GpioPinWriteSuccess(pin);
    return true;
}

GpioStatus GR740GpioDriver::gpioRead_handler(const FwIndexType portNum, Fw::Logic& state) {
    if (!m_initialized) {
        this->initialize();
        if (!m_initialized) {
            this->log_WARNING_HI_GpioPinReadError(portNum);
            return GpioStatus::NOT_OPENED;
        }
    }

    // Check if the port is in the valid range
    if (static_cast<U32>(portNum) >= MAX_GPIO_PINS) {
        this->log_WARNING_HI_GpioPinReadError(portNum);
        return GpioStatus::INVALID_MODE;
    }

    // Determine which GPIO controller and bit to use based on portNum
    volatile uint32_t* data_reg = nullptr;
    uint32_t pin_mask = 0;

    // Map portNum to specific GPIO
    switch (portNum) {
        case 0: // LED7 (GPIO2[5])
            data_reg = m_gpio1_data_reg;
            pin_mask = LED7_BIT;
            break;
        case 1: // LED8 (GPIO2[6])
            data_reg = m_gpio1_data_reg;
            pin_mask = LED8_BIT;
            break;
        default:
            // Try to use original pin mapping for other pins
            if (portNum < 4) {
                // For pins 0-3, use GPIO1 controller
                data_reg = m_gpio1_data_reg;
                pin_mask = (1U << portNum);
            } else {
                // For pins 4+, use GPIO0 controller
                data_reg = m_gpio0_data_reg;
                pin_mask = (1U << (portNum - 4));
            }
            break;
    }

    // Read pin value
    state = (*data_reg & pin_mask) ? Fw::Logic::HIGH : Fw::Logic::LOW;

    this->log_DIAGNOSTIC_GpioPinReadSuccess(portNum);
    return GpioStatus::OP_OK;
}

}  // end namespace Drv