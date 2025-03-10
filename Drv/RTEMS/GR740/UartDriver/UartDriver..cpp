// ======================================================================
// \title  UartDriver.cpp
// \author fprime-community
// \brief  cpp file for GR740UartDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/RTEMS/GR740/UartDriver/UartDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>

namespace Drv {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  GR740UartDriver ::
    GR740UartDriver(const char* const compName, const U32 uartInstance) :
      GR740UartDriverComponentBase(compName),
      m_uartInstance(uartInstance),
      m_uartDevice(nullptr),
      m_uartRegs(nullptr),
      m_initialized(false),
      m_configured(false),
      m_receiveTaskRunning(false)
  {
    // Initialize configuration with defaults
    m_config.baudRate = 115200;
    m_config.dataBits = 8;
    m_config.stopBits = 1;
    m_config.parityEnabled = false;
    m_config.parityOdd = false;
    m_config.flowControlEnabled = false;
  }

  void GR740UartDriver ::
    init(const FwIndexType instance)
  {
    GR740UartDriverComponentBase::init(instance);
  }

  GR740UartDriver ::
    ~GR740UartDriver()
  {
    // Ensure receive task is stopped
    if (m_receiveTaskRunning) {
      stopReceiveTask();
    }
  }

  bool GR740UartDriver ::
    initialize()
  {
    if (m_initialized) {
      return true; // Already initialized
    }

    // Initialize driver manager if not already initialized by BSP
    RTEMS::DriverUtil::initializeDriverManager();

    // Find the APBUART device in the system
    m_uartDevice = RTEMS::DriverUtil::findAmbaDevice(
      RTEMS::GAISLER_VENDOR_ID,
      RTEMS::DeviceId::APBUART,
      m_uartInstance
    );

    if (m_uartDevice == nullptr) {
      this->log_WARNING_HI_UartInitError(m_uartInstance, -1);
      return false;
    }

    // Get the APBUART register structure
    struct ambapp_dev* ambapp_dev = (struct ambapp_dev*)m_uartDevice->businfo;
    m_uartRegs = (struct apbuart_regs*)ambapp_dev->apb_slv->start;

    if (m_uartRegs == nullptr) {
      this->log_WARNING_HI_UartInitError(m_uartInstance, -2);
      return false;
    }

    // Initialize the UART controller
    // Reset the controller
    m_uartRegs->ctrl = 0;
    
    // Enable receiver and transmitter
    m_uartRegs->ctrl = APBUART_CTRL_RE | APBUART_CTRL_TE;

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_UartInitSuccess(m_uartInstance);
    
    // Signal that the driver is ready
    this->ready_out(0);
    
    return true;
  }

  bool GR740UartDriver ::
    configure(const UartConfiguration& config)
  {
    Os::LockGuard guard(m_mutex);

    if (!m_initialized) {
      this->log_WARNING_HI_UartConfigError(m_uartInstance, -1);
      return false;
    }

    // Store the configuration
    m_config = config;

    // Calculate the scaler value based on system clock and desired baud rate
    // For GR740, the typical system clock is 50 MHz
    // SCALER = (50,000,000 / (baud * 8)) - 1
    U32 systemClock = 50000000; // 50 MHz - you may need to adjust this based on your system
    U32 scaler = (systemClock / (config.baudRate * 8)) - 1;

    // Configure UART
    U32 ctrl = m_uartRegs->ctrl;
    
    // Set basic control bits (receiver and transmitter enabled)
    ctrl |= APBUART_CTRL_RE | APBUART_CTRL_TE;

    // Configure parity
    if (config.parityEnabled) {
      ctrl |= APBUART_CTRL_PE;  // Enable parity
      if (config.parityOdd) {
        ctrl |= APBUART_CTRL_PS;  // Set odd parity
      } else {
        ctrl &= ~APBUART_CTRL_PS; // Set even parity
      }
    } else {
      ctrl &= ~APBUART_CTRL_PE;  // Disable parity
    }

    // Configure flow control
    if (config.flowControlEnabled) {
      ctrl |= APBUART_CTRL_FL;
    } else {
      ctrl &= ~APBUART_CTRL_FL;
    }

    // Configure data bits (APBUART typically uses 8 bits, so this is ignored in this implementation)
    
    // Apply the configuration
    m_uartRegs->ctrl = ctrl;
    
    // Set the scaler
    m_uartRegs->scaler = scaler;

    // Configuration succeeded
    m_configured = true;
    this->log_DIAGNOSTIC_UartConfigSuccess(m_uartInstance, config.baudRate);
    return true;
  }

  bool GR740UartDriver ::
    startReceiveTask(NATIVE_INT_TYPE priority, NATIVE_INT_TYPE stackSize)
  {
    if (!m_initialized || !m_configured) {
      return false;
    }

    if (m_receiveTaskRunning) {
      return true; // Already running
    }

    // Set running flag
    m_receiveTaskRunning = true;

    // Start the receive task
    Os::TaskString name("UartRecvTask");
    Os::Task::Status status = m_receiveTask.start(name, 
                                                 GR740UartDriver::receiveTaskEntry,
                                                 this,
                                                 priority,
                                                 stackSize);

    return (status == Os::Task::OP_OK);
  }

  void GR740UartDriver ::
    stopReceiveTask()
  {
    if (!m_receiveTaskRunning) {
      return;
    }

    // Clear running flag to signal task to exit
    {
      Os::LockGuard guard(m_mutex);
      m_receiveTaskRunning = false;
    }

    // Wait for task to terminate
    m_receiveTask.join();
  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  Drv::SendStatus GR740UartDriver ::
    drvDataIn_handler(
        const FwIndexType portNum,
        Fw::Buffer &fwBuffer
    )
  {
    Os::LockGuard guard(m_mutex);

    if (!m_initialized || !m_configured) {
      this->log_WARNING_HI_UartSendError(m_uartInstance, -1);
      return Drv::SendStatus::SEND_ERROR;
    }

    // Get buffer parameters
    const U8* data = fwBuffer.getData();
    U32 size = fwBuffer.getSize();

    if (data == nullptr || size == 0) {
      this->log_WARNING_HI_UartSendError(m_uartInstance, -2);
      return Drv::SendStatus::SEND_ERROR;
    }

    // Send the data
    if (!sendBuffer(data, size)) {
      this->log_WARNING_HI_UartSendError(m_uartInstance, -3);
      return Drv::SendStatus::SEND_ERROR;
    }

    // Log success
    this->log_DIAGNOSTIC_UartSendSuccess(m_uartInstance, size);
    return Drv::SendStatus::SEND_OK;
  }

  // ----------------------------------------------------------------------
  // Private methods
  // ----------------------------------------------------------------------

  void GR740UartDriver ::
    receiveTaskEntry(void* arg)
  {
    GR740UartDriver* driver = static_cast<GR740UartDriver*>(arg);
    FW_ASSERT(driver != nullptr);

    driver->receiveTask();
  }

  void GR740UartDriver ::
    receiveTask()
  {
    while (true) {
      // Check if we should exit
      {
        Os::LockGuard guard(m_mutex);
        if (!m_receiveTaskRunning) {
          break;
        }
      }

      // Receive data
      I32 bytesReceived = receiveBuffer(m_receiveBuffer, sizeof(m_receiveBuffer));

      if (bytesReceived > 0) {
        // Create a buffer to hold the received data
        Fw::Buffer recvBuffer = Fw::Buffer(m_receiveBuffer, bytesReceived);
        
        // Send the data to the output port
        this->drvDataOut_out(0, recvBuffer, Drv::RecvStatus::RECV_OK);
        
        // Log success
        this->log_DIAGNOSTIC_UartRecvSuccess(m_uartInstance, bytesReceived);
      } 
      else if (bytesReceived < 0) {
        // Error occurred
        this->log_WARNING_HI_UartRecvError(m_uartInstance, bytesReceived);

        // Create an empty buffer for error reporting
        Fw::Buffer errorBuffer = Fw::Buffer(nullptr, 0);
        
        // Send error status
        this->drvDataOut_out(0, errorBuffer, Drv::RecvStatus::RECV_ERROR);

        // Small delay before retrying to prevent CPU hogging on persistent errors
        Os::Task::delay(10);
      }
      else {
        // No data available, small delay before polling again
        Os::Task::delay(10);
      }
    }
  }

  bool GR740UartDriver ::
    sendBuffer(const U8* buffer, size_t size)
  {
    if (!m_initialized || !m_configured || buffer == nullptr) {
      return false;
    }

    // Send data byte by byte
    for (size_t i = 0; i < size; i++) {
      // Wait for transmitter to be ready
      while (!(m_uartRegs->status & APBUART_STATUS_TE)) {
        // This is a busy-wait loop - in a real driver you might use timeouts
      }

      // Send the byte
      m_uartRegs->data = buffer[i];
    }

    return true;
  }

  I32 GR740UartDriver ::
    receiveBuffer(U8* buffer, size_t size)
  {
    if (!m_initialized || !m_configured || buffer == nullptr || size == 0) {
      return -1;
    }

    I32 bytesReceived = 0;
    
    // Check if data is available
    if (!(m_uartRegs->status & APBUART_STATUS_DR)) {
      return 0; // No data available
    }

    // Read data until either buffer is full or no more data available
    for (size_t i = 0; i < size; i++) {
      // Check if data is available
      if (!(m_uartRegs->status & APBUART_STATUS_DR)) {
        break;
      }

      // Read the byte
      buffer[i] = m_uartRegs->data & 0xFF;
      bytesReceived++;
    }

    return bytesReceived;
  }

} // end namespace Drv