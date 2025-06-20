module Drv {

  @ An active component that uses the GR740 GPTIMER hardware to drive rate groups via ISR
  active component GR740TimerDriver {

    # ----------------------------------------------------------------------
    # General ports
    # ----------------------------------------------------------------------

    @ The cycle output port that drives rate groups
    output port CycleOut: Svc.Cycle

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    @ Time get port
    time get port Time

    @ Telemetry port
    telemetry port Tlm

    @ Event port
    event port Log

    @ Text event port
    text event port LogText

    # ----------------------------------------------------------------------
    # Internal ports
    # ----------------------------------------------------------------------

    @ Internal interrupt reporting interface for GPTIMER ISR
    internal port InterruptReport(
                                   interrupt: U32 @< The interrupt register value
                                 ) \
      priority 1

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------

    @ Timer initialized successfully
    event TimerInitialized(
                           frequency: U32 @< Timer frequency in Hz
                         ) \
      severity activity high \
      id 0x00 \
      format "Timer initialized at {} Hz"

    @ Timer started
    event TimerStarted \
      severity activity high \
      id 0x01 \
      format "Timer started"

    @ Timer stopped
    event TimerStopped \
      severity activity high \
      id 0x02 \
      format "Timer stopped"

    @ Timer error
    event TimerError(
                    errorCode: U32 @< Error code
                   ) \
      severity warning high \
      id 0x03 \
      format "Timer error: {}"

    @ GPTIMER hardware initialized
    event GptimerHwInitialized(
                              timerUnit: U32 @< Timer unit number
                            ) \
      severity activity high \
      id 0x04 \
      format "GPTIMER hardware unit {} initialized"

    @ ISR registered successfully
    event IsrRegistered(
                       timerUnit: U32 @< Timer unit number
                     ) \
      severity activity high \
      id 0x05 \
      format "ISR registered for GPTIMER unit {}"

    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------

    @ Cycles executed
    telemetry TimerCycles: U32 id 0x00

    @ Timer interrupts received
    telemetry TimerInterrupts: U32 id 0x01

    @ Timer frequency in Hz
    telemetry TimerFrequency: U32 id 0x02

  }

}