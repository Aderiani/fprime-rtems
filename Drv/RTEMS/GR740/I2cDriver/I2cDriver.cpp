// ======================================================================
// \title  I2cDriver.cpp
// \author [Your Name]
// \brief  cpp file for I2cDriver component implementation class for GR740 (RTEMS)
//
// \copyright
// Copyright 2025, [Your Organization or Name].
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/RTEMS/GR740/I2cDriver/I2cDriver.hpp>
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>

namespace Drv {

I2cDriver::I2cDriver(const char* const compName)
    : GR740I2cDriverComponentBase(compName), m_baseAddr(nullptr) {}

bool I2cDriver::open() {
    // Set base address from DriverCommon
    m_baseAddr = reinterpret_cast<volatile U32*>(RTEMS::BaseAddress::I2C);
    if (!m_baseAddr) {
        this->log_WARNING_HI_I2cInitError();
        return false;
    }

    // Initialize I2C hardware (e.g., set prescaler and enable)
    m_baseAddr[I2C_PRES] = 0xFFFF; // Example prescaler value (adjust as needed)
    m_baseAddr[I2C_CTRL] = CTRL_EN; // Enable I2C core

    this->log_DIAGNOSTIC_I2cInitSuccess();
    return true;
}

I2cDriver::~I2cDriver() {
    if (m_baseAddr) {
        m_baseAddr[I2C_CTRL] = 0; // Disable I2C core
    }
}

// ----------------------------------------------------------------------
// Handler implementations
// ----------------------------------------------------------------------

I2cStatus I2cDriver::write_handler(const FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) {
    if (!m_baseAddr) {
        return I2cStatus::I2C_WRITE_ERR;
    }

    U8* data = serBuffer.getData();
    U32 size = serBuffer.getSize();

    // Set I2C slave address
    m_baseAddr[I2C_ADDR] = addr << 1; // 7-bit address, shifted for R/W bit

    // Start write operation
    m_baseAddr[I2C_CTRL] = CTRL_EN | CTRL_STA | CTRL_WR;

    // Write data
    for (U32 i = 0; i < size; i++) {
        m_baseAddr[I2C_DATA] = data[i];
        if (!waitForComplete()) {
            m_baseAddr[I2C_CTRL] = CTRL_EN | CTRL_STO; // Stop on error
            return  I2cStatus::I2C_WRITE_ERR;
        }
    }

    // Stop condition
    m_baseAddr[I2C_CTRL] = CTRL_EN | CTRL_STO;
    return waitForComplete() ?  I2cStatus::I2C_OK :  I2cStatus::I2C_WRITE_ERR;
}

I2cStatus I2cDriver::read_handler(const FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) {
    if (!m_baseAddr || serBuffer.getSize() == 0) {
        return  I2cStatus::I2C_ADDRESS_ERR;
    }

    U8* data = serBuffer.getData();
    U32 size = serBuffer.getSize();

    // Set I2C slave address (read mode)
    m_baseAddr[I2C_ADDR] = (addr << 1) | 1; // 7-bit address + read bit

    // Start read operation
    m_baseAddr[I2C_CTRL] = CTRL_EN | CTRL_STA | CTRL_RD;

    // Read data
    for (U32 i = 0; i < size; i++) {
        if (!waitForComplete()) {
            m_baseAddr[I2C_CTRL] = CTRL_EN | CTRL_STO; // Stop on error
            return  I2cStatus::I2C_READ_ERR;
        }
        data[i] = static_cast<U8>(m_baseAddr[I2C_DATA]);
    }

    // Stop condition
    m_baseAddr[I2C_CTRL] = CTRL_EN | CTRL_STO;
    return waitForComplete() ?  I2cStatus::I2C_OK : I2cStatus::I2C_READ_ERR;
}

I2cStatus I2cDriver::writeRead_handler(const FwIndexType portNum, U32 addr,
                                       Fw::Buffer& writeBuffer, Fw::Buffer& readBuffer) {
    if (!m_baseAddr || writeBuffer.getSize() == 0 || readBuffer.getSize() == 0) {
        return I2cStatus::I2C_ADDRESS_ERR;
    }

    // First, perform the write
    I2cStatus status = write_handler(portNum, addr, writeBuffer);
    if (status !=  I2cStatus::I2C_OK) {
        return status;
    }

    // Then, perform the read
    return read_handler(portNum, addr, readBuffer);
}

// ----------------------------------------------------------------------
// Private helper functions
// ----------------------------------------------------------------------

bool I2cDriver::waitForComplete() {
    U32 timeout = 10000; // Adjust timeout as needed
    while (timeout--) {
        if (m_baseAddr[I2C_STAT] & STAT_DONE) {
            return !(m_baseAddr[I2C_STAT] & STAT_BUSY); // Success if not busy
        }
    }
    this->log_WARNING_HI_I2cTransactionError();
    return false;
}

} // end namespace Drv