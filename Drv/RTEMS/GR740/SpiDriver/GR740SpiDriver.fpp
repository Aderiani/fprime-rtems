module Drv {

  @ A driver component for GR740 SPI controller
  passive component GR740SpiDriver {

    include "../../../Interfaces/SpiInterface.fppi"

    # ----------------------------------------------------------------------
    # Special ports 
    # ----------------------------------------------------------------------

    event port Log

    telemetry port Tlm

    text event port LogText

    time get port Time


    @ Bytes Sent/Received
    telemetry SPI_Bytes: U64 id 0



    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
@ SPI open error
event SPI_OpenError(
                     select: I32 @< The chip select
                     error: I32 @< The error code
                   ) \
  severity warning high \
  id 0 \
  format "Error opening SPI device {}: {}"

@ SPI config error
event SPI_ConfigError(
                       select: I32 @< The chip select
                       error: I32 @< The error code
                     ) \
  severity warning high \
  id 1 \
  format "Error configuring SPI device {}: {}"

@ SPI write error
event SPI_WriteError(
                      select: I32 @< The chip select
                      error: I32 @< The error code
                    ) \
  severity warning high \
  id 2 \
  format "Error writing/reading SPI device {}: {}" \
  throttle 5

@ SPI open notification
event SPI_PortOpened(
                      select: I32 @< The chip select
                    ) \
  severity activity high \
  id 4 \
  format "SPI Device {} configured"


  }

}