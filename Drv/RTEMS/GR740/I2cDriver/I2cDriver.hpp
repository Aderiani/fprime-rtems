// ======================================================================
// \title  I2cDriver.hpp
// \author fprime-community
// \brief  hpp file for GR740I2cDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_GR740_I2C_DRIVER_HPP
#define DRV_GR740_I2C_DRIVER_HPP

#include <Drv/RTEMS/GR740/I2cDriver/I2cDriverComponentAc.hpp>
#include <Drv/RTEMS/include/DriverCommon.hpp>

// GRLIB I2CMST driver includes
#include <grlib/i2cmst.h>

namespace Drv {

  class GR740I2cDriver final : public GR740I2cDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object GR740I2cDriver
    //!
    GR740I2cDriver(
        const char* const compName /*!< The component name*/
    );

    //! Destroy object GR740I2cDriver
    //!
    ~GR740I2cDriver();

    //! \brief Initialize the I2C driver
    //!
    //! This function initializes the I2C driver by finding the I2CMST device
    //! in the system and setting up the driver.
    //!
    //! \param instance The instance number of the I2CMST device (default: 0)
    //! \param clockFreqKhz The I2C clock frequency in kHz (default: 100 kHz)
    //! \return true if initialization was successful, false otherwise
    bool initialize(NATIVE_INT_TYPE instance = 0, NATIVE_UINT_TYPE clockFreqKhz = 100);

  PRIVATE:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for write
    //!
    I2cStatus write_handler(
        const FwIndexType portNum, /*!< The port number*/
        U32 addr, /*!< The I2C device address*/
        Fw::Buffer &serBuffer /*!< Buffer containing data to write*/
    ) override;

    //! Handler implementation for read
    //!
    I2cStatus read_handler(
        const FwIndexType portNum, /*!< The port number*/
        U32 addr, /*!< The I2C device address*/
        Fw::Buffer &serBuffer /*!< Buffer to store read data*/
    ) override;

    //! Handler implementation for writeRead
    //!
    I2cStatus writeRead_handler(
        const FwIndexType portNum, /*!< The port number*/
        U32 addr, /*!< The I2C device address*/
        Fw::Buffer &writeBuffer, /*!< Buffer containing data to write*/
        Fw::Buffer &readBuffer /*!< Buffer to store read data*/
    ) override;

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------
    struct drvmgr_dev* m_i2cDevice; //!< I2CMST device pointer
    I2CMST_Device m_i2cRegs;       //!< I2CMST register structure
    bool m_initialized;             //!< Initialization flag
  };

} // end namespace Drv

#endif