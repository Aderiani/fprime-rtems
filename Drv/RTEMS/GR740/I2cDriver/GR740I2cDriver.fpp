module Drv {

  @ A driver component for GR740 I2C controller
  passive component GR740I2cDriver {

    include "../../../Interfaces/I2cInterface.fppi"

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    event port Log

    text event port LogText

    time get port Time

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    event I2cInitSuccess() severity diagnostic format "GR740 I2C driver initialized successfully"

    event I2cInitError() severity warning high format "Failed to initialize GR740 I2C driver"

    event I2cTransactionSuccess() severity diagnostic format "Successful I2C transaction with device"

    event I2cTransactionError() severity warning high format "I2C transaction with device failed"

  }

}