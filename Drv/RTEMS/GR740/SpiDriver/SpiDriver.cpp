// ======================================================================
// \title  SpiDriver.cpp
// \author fprime-community
// \brief  cpp file for GR740SpiDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/RTEMS/GR740/SpiDriver/SpiDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>

namespace Drv {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  GR740SpiDriver ::
    GR740SpiDriver(const char* const compName) :
      GR740SpiDriverComponentBase(compName),
      m_spiDevice(nullptr),
      m_spiRegs(nullptr),
      m_initialized(false),
      m_currentDevice(0)
  {
    // Initialize device configurations with defaults
    for (NATIVE_UINT_TYPE i = 0; i < FW_NUM_ARRAY_ELEMENTS(m_deviceConfigs); i++) {
      m_deviceConfigs[i].frequency = 1000000;  // 1 MHz default
      m_deviceConfigs[i].mode = 0;             // Mode 0 default
      m_deviceConfigs[i].bitsPerWord = 8;      // 8 bits default
      m_deviceConfigs[i].lsbFirst = false;     // MSB first default
      m_deviceConfigs[i].chipSelectActive = false; // CS active low default
    }
  }

  GR740SpiDriver ::
    ~GR740SpiDriver()
  {
    // No cleanup needed as the driver manager handles device resources
  }

  bool GR740SpiDriver ::
    initialize(NATIVE_INT_TYPE instance)
  {
    if (m_initialized) {
      return true; // Already initialized
    }

    // Initialize driver manager if not already initialized by BSP
    RTEMS::DriverUtil::initializeDriverManager();

    // Find the SPICTRL device in the system
    m_spiDevice = RTEMS::DriverUtil::findAmbaDevice(
      RTEMS::GAISLER_VENDOR_ID,
      RTEMS::DeviceId::SPICTRL,
      instance
    );

    if (m_spiDevice == nullptr) {
      this->log_WARNING_HI_SpiInitError(-1);
      return false;
    }

    // Get the SPICTRL register structure
    struct ambapp_dev* ambapp_dev = (struct ambapp_dev*)m_spiDevice->businfo;
    m_spiRegs = (struct spictrl_regs*)ambapp_dev->apb_slv->start;

    if (m_spiRegs == nullptr) {
      this->log_WARNING_HI_SpiInitError(-2);
      return false;
    }

    // Initialize the SPI controller
    // Reset the controller
    m_spiRegs->mode = SPICTRL_MODE_RST;
    
    // Clear reset bit
    m_spiRegs->mode = 0;

    // Initialize success
    m_initialized = true;
    this->log_DIAGNOSTIC_SpiInitSuccess();
    return true;
  }

  bool GR740SpiDriver ::
    configureDevice(
      NATIVE_UINT_TYPE device,
      const SpiDeviceConfiguration& config
    )
  {
    if (!m_initialized) {
      this->log_WARNING_HI_SpiConfigureError(device, -1);
      return false;
    }

    // Verify device number is valid
    if (device >= FW_NUM_ARRAY_ELEMENTS(m_deviceConfigs)) {
      this->log_WARNING_HI_SpiConfigureError(device, -2);
      return false;
    }

    // Store the configuration
    m_deviceConfigs[device] = config;

    // Configuration succeeded
    this->log_DIAGNOSTIC_SpiConfigureSuccess(device);
    return true;
  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void GR740SpiDriver ::
    spiIn_handler(
        const FwIndexType portNum,
        Drv::SpiStatus &status,
        Fw::Buffer &recvBuffer,
        const Fw::Buffer &sendBuffer
    )
  {
    status = Drv::SPI_ERROR;

    if (!m_initialized) {
      this->log_WARNING_HI_SpiTransferError(m_currentDevice, -1);
      return;
    }

    // Check buffer parameters
    if (sendBuffer.getSize() == 0 || 
        (recvBuffer.getSize() > 0 && recvBuffer.getSize() < sendBuffer.getSize())) {
      this->log_WARNING_HI_SpiTransferError(m_currentDevice, -2);
      return;
    }

    // Get the device number from the port number
    NATIVE_UINT_TYPE device = static_cast<NATIVE_UINT_TYPE>(portNum);
    if (device >= FW_NUM_ARRAY_ELEMENTS(m_deviceConfigs)) {
      this->log_WARNING_HI_SpiTransferError(device, -3);
      return;
    }

    // Configure the SPI controller for this device
    U32 mode = 0;
    
    // Set clock phase and polarity based on SPI mode
    switch (m_deviceConfigs[device].mode) {
      case 0: // CPOL=0, CPHA=0
        mode &= ~(SPICTRL_MODE_CPOL | SPICTRL_MODE_CPHA);
        break;
      case 1: // CPOL=0, CPHA=1
        mode &= ~SPICTRL_MODE_CPOL;
        mode |= SPICTRL_MODE_CPHA;
        break;
      case 2: // CPOL=1, CPHA=0
        mode |= SPICTRL_MODE_CPOL;
        mode &= ~SPICTRL_MODE_CPHA;
        break;
      case 3: // CPOL=1, CPHA=1
        mode |= (SPICTRL_MODE_CPOL | SPICTRL_MODE_CPHA);
        break;
      default:
        this->log_WARNING_HI_SpiTransferError(device, -4);
        return;
    }
    
    // Set LSB/MSB first
    if (m_deviceConfigs[device].lsbFirst) {
      mode |= SPICTRL_MODE_LSB;
    } else {
      mode &= ~SPICTRL_MODE_LSB;
    }
    
    // Set bits per word
    if (m_deviceConfigs[device].bitsPerWord != 8) {
      // Only 8 bits per word supported in this implementation
      this->log_WARNING_HI_SpiTransferError(device, -5);
      return;
    }
    
    // Set the SPI clock frequency (approximated - actual calculation would need more details)
    // This is a simplified version - in a real driver you'd calculate the divider value
    U32 clockDivider = 1; // This should be calculated based on system clock and desired frequency
    mode |= ((clockDivider & 0xFF) << 16);
    
    // Set the mode register
    m_spiRegs->mode = mode;
    
    // Select the device (chip select)
    // In the GR740, each device has its own chip select line
    m_spiRegs->mask = (1 << device);
    if (m_deviceConfigs[device].chipSelectActive) {
      m_spiRegs->cs = (1 << device); // CS active high
    } else {
      m_spiRegs->cs = 0;             // CS active low
    }
    
    // Perform the transfer
    const U8* sendData = sendBuffer.getData();
    U8* recvData = (recvBuffer.getSize() > 0) ? recvBuffer.getData() : nullptr;
    NATIVE_UINT_TYPE transferSize = sendBuffer.getSize();
    NATIVE_UINT_TYPE transferredBytes = 0;
    
    // Transfer data byte by byte (could be optimized with FIFO/DMA if supported)
    for (NATIVE_UINT_TYPE i = 0; i < transferSize; i++) {
      // Write the data to transmit
      m_spiRegs->tx = sendData[i];
      
      // Wait for transfer to complete
      while (!(m_spiRegs->status & SPICTRL_STATUS_TRDY)) {
        // This is a busy-wait loop - in a real driver you might use interrupts or timeouts
      }
      
      // Read the received data if needed
      if (recvData) {
        recvData[i] = m_spiRegs->rx & 0xFF;
      } else {
        // Dummy read to clear the RX register
        volatile U32 dummy = m_spiRegs->rx;
        (void)dummy;
      }
      
      transferredBytes++;
    }
    
    // Deselect the device
    m_spiRegs->mask = 0;
    m_spiRegs->cs = 0;
    
    // Record the last used device
    m_currentDevice = device;
    
    // Update the receive buffer size if used
    if (recvData) {
      recvBuffer.setSize(transferredBytes);
    }
    
    // Log success and set status
    this->log_DIAGNOSTIC_SpiTransferSuccess(device, transferredBytes);
    status = Drv::SPI_OK;
  }

} // end namespace Drv