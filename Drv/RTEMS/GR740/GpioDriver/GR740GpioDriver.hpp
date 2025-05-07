#ifndef DRV_GR740_GPIO_DRIVER_HPP
#define DRV_GR740_GPIO_DRIVER_HPP

// Include the component-specific generated header
#include <Drv/RTEMS/GR740/GpioDriver/GR740GpioDriverComponentAc.hpp>

// Include our driver common definitions
#include <Drv/RTEMS/include/DriverCommon.hpp>

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
    bool m_initialized;              //!< Initialization flag
    Fw::Logic m_outputState[MAX_GPIO_PINS]; //!< Current output state of each pin
    GpioDirection m_pinDirection[MAX_GPIO_PINS]; //!< Direction configuration for each pin
    
    // Direct register access pointers for both GPIO controllers
    volatile uint32_t* m_gpio0_data_reg;  //!< GPIO0 data register
    volatile uint32_t* m_gpio0_dir_reg;   //!< GPIO0 direction register
    volatile uint32_t* m_gpio1_data_reg;  //!< GPIO1 data register
    volatile uint32_t* m_gpio1_dir_reg;   //!< GPIO1 direction register
    
    // Constants for GPIO controller addresses
    static constexpr uintptr_t GPIO0_BASE_ADDR = 0xffa08000;
    static constexpr uintptr_t GPIO1_BASE_ADDR = 0xff902000;
    static constexpr uintptr_t GPIO_DATA_OUT_OFFSET = 0x04;
    static constexpr uintptr_t GPIO_DIRECTION_OFFSET = 0x08;
    
    // LED pin mapping
    static constexpr uint32_t LED7_BIT = 0x20;  // Bit 5 for LED7
    static constexpr uint32_t LED8_BIT = 0x40;  // Bit 6 for LED8
  };

} // end namespace Drv

#endif // DRV_GR740_GPIO_DRIVER_HPP