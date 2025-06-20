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


    // Initialize success
    m_initialized = true;
    // this->log_DIAGNOSTIC_GpioInitSuccess();
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

    // For LED7 and LED8, we need to write to BOTH controllers as shown in gpio_test.c
    uint32_t pin_mask = 0;
    
    // Map portNum to LED pin mask
    switch (portNum) {
        case 0: // LED7
            pin_mask = LED7_BIT;
            break;
        case 1: // LED8
            pin_mask = LED8_BIT;
            break;
        default:
            // For other pins, use original mapping
            if (portNum < 4) {
                pin_mask = (1U << portNum);
            } else {
                pin_mask = (1U << (portNum - 4));
            }
            break;
    }

    // For LEDs, write to BOTH GPIO controllers (matching gpio_test.c behavior)
    if (portNum == 0 || portNum == 1) {
        // Set direction bits on both controllers
        *m_gpio0_dir_reg |= pin_mask;
        *m_gpio1_dir_reg |= pin_mask;
        
        // Set or clear the LED bit on both controllers
        if (state == Fw::Logic::HIGH) {
            *m_gpio0_data_reg |= pin_mask;  // Set bit on GPIO0
            *m_gpio1_data_reg |= pin_mask;  // Set bit on GPIO1
            
        } else {
            *m_gpio0_data_reg &= ~pin_mask; // Clear bit on GPIO0
            *m_gpio1_data_reg &= ~pin_mask; // Clear bit on GPIO1
            
        }
    } else {
        // For non-LED pins, use the original logic
        volatile uint32_t* data_reg = nullptr;
        volatile uint32_t* dir_reg = nullptr;
        
        if (portNum < 4) {
            data_reg = m_gpio1_data_reg;
            dir_reg = m_gpio1_dir_reg;
        } else {
            data_reg = m_gpio0_data_reg;
            dir_reg = m_gpio0_dir_reg;
        }
        
        // Ensure pin is configured as output
        *dir_reg |= pin_mask;
        
        // Set or clear the bit
        if (state == Fw::Logic::HIGH) {
            *data_reg |= pin_mask;
        } else {
            *data_reg &= ~pin_mask;
        }
    }

    // Store current state
    m_outputState[portNum] = state;

    // this->log_DIAGNOSTIC_GpioPinWriteSuccess(portNum);
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

    // For LED pins, configure both controllers
    if (pin == 0 || pin == 1) {
        uint32_t pin_mask = (pin == 0) ? LED7_BIT : LED8_BIT;
        
        if (direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
            // Set as output on both controllers
            *m_gpio0_dir_reg |= pin_mask;
            *m_gpio1_dir_reg |= pin_mask;
            
            // Set initial value on both controllers
            if (initialValue == Fw::Logic::HIGH) {
                *m_gpio0_data_reg |= pin_mask;
                *m_gpio1_data_reg |= pin_mask;
                m_outputState[pin] = Fw::Logic::HIGH;
            } else {
                *m_gpio0_data_reg &= ~pin_mask;
                *m_gpio1_data_reg &= ~pin_mask;
                m_outputState[pin] = Fw::Logic::LOW;
            }
            

        } else {
            // Set as input on both controllers
            *m_gpio0_dir_reg &= ~pin_mask;
            *m_gpio1_dir_reg &= ~pin_mask;
        }
    } else {
        // For non-LED pins, use original logic
        volatile uint32_t* data_reg = nullptr;
        volatile uint32_t* dir_reg = nullptr;
        uint32_t pin_mask = 0;
        
        if (pin < 4) {
            data_reg = m_gpio1_data_reg;
            dir_reg = m_gpio1_dir_reg;
            pin_mask = (1U << pin);
        } else {
            data_reg = m_gpio0_data_reg;
            dir_reg = m_gpio0_dir_reg;
            pin_mask = (1U << (pin - 4));
        }
        
        // Configure pin direction
        if (direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
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
            // Set as input
            *dir_reg &= ~pin_mask;
        }
    }

    // Store pin direction
    m_pinDirection[pin] = direction;

    // this->log_DIAGNOSTIC_GpioPinWriteSuccess(pin);
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

    // For LEDs, read from GPIO0 (primary controller)
    volatile uint32_t* data_reg = nullptr;
    uint32_t pin_mask = 0;

    switch (portNum) {
        case 0: // LED7
            data_reg = m_gpio0_data_reg;  // Read from GPIO0
            pin_mask = LED7_BIT;
            break;
        case 1: // LED8
            data_reg = m_gpio0_data_reg;  // Read from GPIO0
            pin_mask = LED8_BIT;
            break;
        default:
            // For other pins, use original mapping
            if (portNum < 4) {
                data_reg = m_gpio1_data_reg;
                pin_mask = (1U << portNum);
            } else {
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