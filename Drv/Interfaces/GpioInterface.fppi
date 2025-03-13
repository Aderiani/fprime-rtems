module Drv {
  enum GpioDirection {
    GPIO_IN = 0
    GPIO_OUT = 1
  }

  enum GpioStatus {
    GPIO_OK = 0
    GPIO_ERROR = 1
    GPIO_INVALID_PIN = 2
    GPIO_INVALID_MODE = 3
    GPIO_INVALID_DIRECTION = 4
  }

  port GpioWrite(
    state: Fw.Logic
  ) -> GpioStatus

  port GpioRead(
    ref state: Fw.Logic
  ) -> GpioStatus
}

@ Port used to write to a GPIO pin
sync input port gpioWrite: Drv.GpioWrite

@ Port used to read from a GPIO pin
sync input port gpioRead: Drv.GpioRead

@ Port used to indicate transition on the GPIO pin
output port gpioInterrupt: Svc.Cycle