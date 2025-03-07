module Drv {

  @ A GPIO driver for the GR740 board
  passive component GR740GpioDriver {

    # ----------------------------------------------------------------------
    # General ports
    # ----------------------------------------------------------------------

    include "../../../Interfaces/GpioInterface.fppi"

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    @ Event port
    event port Log

    @ Text event port
    text event port LogText

    @ Time get port
    time get port Time

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    
    @ Error setting GPIO direction
    event DirectionError(
                         gpio: U32 @< The GPIO number
                         direction: I32 @< The direction requested
                         error: I32 @< The error code
                        ) \
      severity warning high \
      format "Failed to set GPIO {}: direction {} error {}"

    @ Error reading GPIO value
    event ReadError(
                    gpio: U32 @< The GPIO number
                    error: I32 @< The error code
                   ) \
      severity warning high \
      format "Failed to read GPIO {}: error {}"

    @ Error writing GPIO value
    event WriteError(
                     gpio: U32 @< The GPIO number
                     value: Fw.Logic @< The value being written
                     error: I32 @< The error code
                    ) \
      severity warning high \
      format "Failed to write GPIO {}: value {} error {}"

  }

}