module Drv {

  @ A driver component for GR740 UART controller
  active component GR740UartDriver {

    # ----------------------------------------------------------------------
    # General ports
    # ----------------------------------------------------------------------

    @ Byte stream driver model input
    async input port drvDataIn: Drv.ByteStreamSend

    @ Byte stream driver model output
    output port drvDataOut: Drv.ByteStreamRecv
    
    @ Port indicating the driver is ready to receive data
    output port ready: Drv.ByteStreamReady

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    event port Log

    text event port LogText

    time get port Time

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    event UartInitSuccess(device: U32) severity diagnostic format "GR740 UART{} driver initialized successfully"

    event UartInitError(device: U32, error: I32) severity warning high format "Failed to initialize GR740 UART{} driver with error code {}"

    event UartConfigSuccess(device: U32, baud: U32) severity diagnostic format "Successfully configured UART{} with baud rate {}"

    event UartConfigError(device: U32, error: I32) severity warning high format "Failed to configure UART{} with error code {}"

    event UartSendSuccess(device: U32, bytes: U32) severity diagnostic format "Successfully sent {} bytes on UART{}"

    event UartSendError(device: U32, error: I32) severity warning high format "UART{} send failed with error code {}"

    event UartRecvSuccess(device: U32, bytes: U32) severity diagnostic format "Successfully received {} bytes on UART{}"

    event UartRecvError(device: U32, error: I32) severity warning high format "UART{} receive failed with error code {}"

  }

}