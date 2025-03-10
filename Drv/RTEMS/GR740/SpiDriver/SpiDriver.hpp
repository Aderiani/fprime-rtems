// ======================================================================
// \title  SpiDriver.hpp
// \author fprime-community
// \brief  hpp file for GR740SpiDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_GR740_SPI_DRIVER_HPP
#define DRV_GR740_SPI_DRIVER_HPP

#include <Drv/RTEMS/GR740/SpiDriver/SpiDriverComponentAc.hpp>
#include <Drv/RTEMS/include/DriverCommon.hpp>

// GRLIB SPICTRL driver includes
#include <grlib/spictrl.h>

namespace Drv {

  class GR740SpiDriver final : public GR740SpiDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object GR740SpiDriver
    //!
    GR740SpiDriver(
        const char* const compName /*!< The component name*/
    );

    //! Destroy object GR740SpiDriver
    //!
    ~GR740SpiDriver();

    // ----------------------------------------------------------------------
    // SPI configuration types
    // ----------------------------------------------------------------------
    
    struct SpiDeviceConfiguration {
      U32 frequency;           //!< SPI clock frequency in Hz
      U8 mode;                 //!< SPI mode (0-3)
      U8 bitsPerWord;          //!< Bits per word (typically 8)
      bool lsbFirst;           //!< LSB first (true) or MSB first (false)
      bool chipSelectActive;   //!< CS active high (true) or low (false)
    };

    //! \brief Initialize the SPI driver
    //!
    //! This function initializes the SPI driver by finding the SPICTRL device
    //! in the system and setting up the driver.
    //!
    //! \param instance The instance number of the SPICTRL device (default: 0)
    //! \return true if initialization was successful, false otherwise
    bool initialize(NATIVE_INT_TYPE instance = 0);

    //! \brief Configure an SPI device
    //!
    //! \param device SPI device number
    //! \param config SPI device configuration
    //! \return true if configuration was successful, false otherwise
    bool configureDevice(
      NATIVE_UINT_TYPE device,
      const SpiDeviceConfiguration& config
    );

  PRIVATE:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for spiIn
    //!
    void spiIn_handler(
        const FwIndexType portNum, /*!< The port number*/
        Drv::SpiStatus &status,
        Fw::Buffer &recvBuffer,
        const Fw::Buffer &sendBuffer
    ) override;

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------
    struct drvmgr_dev* m_spiDevice;       //!< SPICTRL device pointer
    struct spictrl_regs* m_spiRegs;       //!< SPICTRL register structure
    bool m_initialized;                    //!< Initialization flag
    U32 m_currentDevice;                   //!< Currently selected device
    SpiDeviceConfiguration m_deviceConfigs[4]; //!< Configuration for up to 4 SPI devices
  };

} // end namespace Drv

#endif