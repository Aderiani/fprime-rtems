module LedBlinker {

  # ----------------------------------------------------------------------
  # Defaults
  # ----------------------------------------------------------------------

  module Default {
    constant QUEUE_SIZE = 20
    constant STACK_SIZE = 64 * 1024
  }

  # ----------------------------------------------------------------------
  # Active component instances
  # ----------------------------------------------------------------------
  # instance led: Components.Led base id 0x0E00 \
  #   queue size Default.QUEUE_SIZE \
  #   stack size Default.STACK_SIZE \
  #   priority 95
  
  instance timerDriver: Drv.GR740TimerDriver base id 0x0100 \



  instance rateGroup1: Svc.ActiveRateGroup base id 0x0200 \
    queue size Default.QUEUE_SIZE \
    stack size 16 * 1024 \
    priority 120

  instance rateGroup2: Svc.ActiveRateGroup base id 0x0300 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 119

  instance rateGroup3: Svc.ActiveRateGroup base id 0x0400 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 118

  instance cmdDisp: Svc.CommandDispatcher base id 0x0500 \
    queue size 20 \ 
    stack size Default.STACK_SIZE *2\
    priority 101

  instance cmdSeq: Svc.CmdSequencer base id 0x0600 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 100

  instance comQueue: Svc.ComQueue base id 0x0700 \
      queue size 50 \
      stack size Default.STACK_SIZE \
      priority 100 \

  instance fileDownlink: Svc.FileDownlink base id 0x0800 \
    queue size 30 \
    stack size Default.STACK_SIZE \
    priority 100

  instance fileManager: Svc.FileManager base id 0x0900 \
    queue size 30 \
    stack size Default.STACK_SIZE \
    priority 100

  instance fileUplink: Svc.FileUplink base id 0x0A00 \
    queue size 30 \
    stack size Default.STACK_SIZE \
    priority 100

  instance eventLogger: Svc.ActiveLogger base id 0x0B00 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 98

  # comment in Svc.TlmChan or Svc.TlmPacketizer
  # depending on which form of telemetry downlink
  # you wish to use

  # instance tlmSend: Svc.TlmChan base id 0x0C00 \
  #   queue size Default.QUEUE_SIZE \
  #   stack size Default.STACK_SIZE \
  #   priority 97

  #instance tlmSend: Svc.TlmPacketizer base id 0x0C00 \
  #    queue size Default.QUEUE_SIZE \
  #    stack size Default.STACK_SIZE \
  #    priority 97

  # instance prmDb: Svc.PrmDb base id 0x0D00 \
  #   queue size Default.QUEUE_SIZE \
  #   stack size Default.STACK_SIZE \
  #   priority 96


  # ----------------------------------------------------------------------
  # Queued component instances
  # ----------------------------------------------------------------------

  instance $health: Svc.Health base id 0x2000 \
    queue size 25

  # ----------------------------------------------------------------------
  # Passive component instances
  # ----------------------------------------------------------------------

  instance network: Drv.GR740NetworkComponent base id 0x1000
  
  instance tcpServer: Drv.TcpServer base id 0x3000
  
  instance framer: Svc.Framer base id 0x4100

  instance fatalAdapter: Svc.AssertFatalAdapter base id 0x4200

  instance fatalHandler: Svc.FatalHandler base id 0x4300

  instance bufferManager: Svc.BufferManager base id 0x4400

#  instance chronoTime: Svc.ChronoTime base id 0x4500

  instance rateGroupDriver: Svc.RateGroupDriver base id 0x4600

  # instance textLogger: Svc.PassiveTextLogger base id 0x4800

  instance deframer: Svc.Deframer base id 0x4900

  # instance systemResources: Svc.SystemResources base id 0x4A00

  instance comStub: Svc.ComStub base id 0x4B00

  instance gpioDriver: Drv.GR740GpioDriver base id 0x4C00

  instance uartDriver: Drv.GR740UartDriver base id 0x4D00

  instance spiDriver: Drv.GR740SpiDriver base id 0x4E00

  instance watchdogDriver: Drv.GR740WatchdogDriver base id 0x4F00


}
