// ======================================================================
// \title  SpiDriver.cpp
// \author [Your Name]
// \brief  cpp file for SpiDriver component implementation class for GR740 (RTEMS)
//
// \copyright
// Copyright 2025, [Your Organization or Name].
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/RTEMS/GR740/SpiDriver/SpiDriver.hpp>
#include <Fw/Types/Assert.hpp>

namespace Drv {

SpiDriver::SpiDriver(const char* const compName)
    : GR740SpiDriverComponentBase(compName), m_baseAddr(nullptr), m_select(-1), m_isOpen(false), m_bytes(0) {}

void SpiDriver::init(const NATIVE_INT_TYPE instance) {
    GR740SpiDriverComponentBase::init(instance);
}

bool SpiDriver::open(NATIVE_INT_TYPE select, SpiFrequency clock, SpiMode spiMode) {
    if (m_isOpen) {
        return true; // Already open
    }

    // Set base address from DriverCommon.hpp
    m_baseAddr = reinterpret_cast<volatile U32*>(RTEMS::BaseAddress::SPI);
    if (!m_baseAddr) {
        this->log_WARNING_HI_SPI_OpenError(select, -1);
        return false;
    }

    m_select = select;

    // Configure SPI mode (CPOL/CPHA)
    U8 modeBits = 0;
    switch (spiMode) {
        case SPI_MODE_CPOL_LOW_CPHA_LOW:
            modeBits = 0; // CPOL=0, CPHA=0
            break;
        case SPI_MODE_CPOL_LOW_CPHA_HIGH:
            modeBits = CTRL_CPHA; // CPOL=0, CPHA=1
            break;
        case SPI_MODE_CPOL_HIGH_CPHA_LOW:
            modeBits = CTRL_CPOL; // CPOL=1, CPHA=0
            break;
        case SPI_MODE_CPOL_HIGH_CPHA_HIGH:
            modeBits = CTRL_CPOL | CTRL_CPHA; // CPOL=1, CPHA=1
            break;
        default:
            FW_ASSERT(0, static_cast<NATIVE_INT_TYPE>(spiMode)); // Invalid mode
            break;
    }

    // Calculate prescaler from clock frequency (assuming 50 MHz system clock)
    // Example: prescaler = (system_clock / desired_clock) - 1
    U32 prescaler = (50000000 / clock) - 1; // Adjust system clock as needed
    m_baseAddr[SPI_PRESC] = prescaler;

    // Initialize SPI hardware (master mode, mode bits)
    m_baseAddr[SPI_CTRL] = CTRL_EN | CTRL_MSTR | modeBits;

    m_isOpen = true;
    this->log_ACTIVITY_HI_SPI_PortOpened(reinterpret_cast<U32>(m_baseAddr));
    return true;
}

SpiDriver::~SpiDriver() {
    if (m_baseAddr && m_isOpen) {
        m_baseAddr[SPI_CTRL] = 0; // Disable SPI core
    }
}

// ----------------------------------------------------------------------
// Handler implementations
// ----------------------------------------------------------------------

void SpiDriver::SpiReadWrite_handler(const NATIVE_INT_TYPE portNum, Fw::Buffer& writeBuffer, Fw::Buffer& readBuffer) {
    if (!m_isOpen) {
        this->log_WARNING_HI_SPI_OpenError(m_select, -1); // Not opened
        return;
    }
    if (writeBuffer.getSize() != readBuffer.getSize()) {
        this->log_WARNING_HI_SPI_OpenError(m_select, -2); // Mismatched buffer sizes
        return;
    }
    if (writeBuffer.getSize() == 0) {
        this->log_WARNING_HI_SPI_WriteError( m_select, -3); // Empty buffer
        return;
    }

    U8* writeData = writeBuffer.getData();
    U8* readData = readBuffer.getData();
    U32 size = writeBuffer.getSize();

    // Set slave select
    m_baseAddr[SPI_SSEL] = 1 << m_select;

    // Start full-duplex transfer
    m_baseAddr[SPI_CTRL] |= CTRL_TXEN | CTRL_RXEN;

    // Perform transfer
    for (U32 i = 0; i < size; i++) {
      m_baseAddr[SPI_DATA] = writeData[i];
      if (!waitForComplete()) {
          m_baseAddr[SPI_SSEL] = 0; // Deselect slave on error
          this->log_WARNING_HI_SPI_WriteError(m_select, -1); // Generic error code
          return;
      }
      readData[i] = static_cast<U8>(m_baseAddr[SPI_DATA]); // Extract lower 8 bits
  }

    // Deselect slave
    m_baseAddr[SPI_SSEL] = 0;

    // Update telemetry
    m_bytes += readBuffer.getSize();
    this->tlmWrite_SPI_Bytes(m_bytes);
}

// ----------------------------------------------------------------------
// Private helper functions
// ----------------------------------------------------------------------

bool SpiDriver::waitForComplete() {
    U32 timeout = 10000; // Adjust timeout as needed
    while (timeout--) {
        U32 status = m_baseAddr[SPI_STAT];
        if (status & STAT_DONE) {
            if (status & STAT_ERR) {
                return false; // Error condition
            }
            return !(status & STAT_BUSY); // Success if not busy
        }
    }
    return false; // Timeout
}

} // end namespace Drv