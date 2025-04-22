module Drv {

  @ A component for servicing the GR740 hardware watchdog timer
  passive component GR740WatchdogDriver {

    # ----------------------------------------------------------------------
    # General ports
    # ----------------------------------------------------------------------

    @ Input port to trigger watchdog service
    sync input port StrokeWatchdog: Svc.Sched

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    @ Time get port
    time get port Time

    @ Event port
    event port Log

    @ Text event port
    text event port LogText

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------

    @ Watchdog initialized successfully
    event WatchdogInitialized(
                             timeout: U32 @< Timeout in milliseconds
                           ) \
      severity activity high \
      id 0x00 \
      format "Watchdog initialized with {} ms timeout"

    @ Watchdog service called
    event WatchdogStroked \
      severity diagnostic \
      id 0x01 \
      format "Watchdog stroked"

  }

}