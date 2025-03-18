module Drv {


  @ A driver component for GR740 GPIO controller
  passive component GR740GpioDriver {
    # include "../../../Ports/GpioDriverPorts.fpp"
    include "../../../Interfaces/GpioInterface.fppi"
  

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    event port Log

    text event port LogText

    time get port Time

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    event GpioInitSuccess() severity diagnostic format "GR740 GPIO driver initialized successfully"

    event GpioInitError() severity warning high format "Failed to initialize GR740 GPIO driver"

    event GpioPinWriteSuccess(pin: U32) severity diagnostic format "Successfully wrote to GPIO pin {}"

    event GpioPinWriteError(pin: U32) severity warning high format "Failed to write to GPIO pin {}"

    event GpioPinReadSuccess(pin: U32) severity diagnostic format "Successfully read from GPIO pin {}"

    event GpioPinReadError(pin: U32) severity warning high format "Failed to read from GPIO pin {}"

    event GpioInterruptRegistered(pin: U32) severity diagnostic format "Successfully registered interrupt for GPIO pin {}"

    event GpioInterruptError(pin: U32) severity warning high format "Failed to register interrupt for GPIO pin {}"

  }

}