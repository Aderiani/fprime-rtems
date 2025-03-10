module Drv {

  @ A driver component for GR740 SPI controller
  passive component GR740SpiDriver {

    include "../../../Interfaces/SpiInterface.fppi"

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    event port Log

    text event port LogText

    time get port Time

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    event SpiInitSuccess() severity diagnostic format "GR740 SPI driver initialized successfully"

    event SpiInitError(error: I32) severity warning high format "Failed to initialize GR740 SPI driver with error code {}"

    event SpiConfigureSuccess(device: U32) severity diagnostic format "Successfully configured SPI device {}"

    event SpiConfigureError(device: U32, error: I32) severity warning high format "Failed to configure SPI device {} with error code {}"

    event SpiTransferSuccess(device: U32, bytes: U32) severity diagnostic format "Successfully transferred {} bytes on SPI device {}"

    event SpiTransferError(device: U32, error: I32) severity warning high format "SPI transfer failed on device {} with error code {}"

  }

}