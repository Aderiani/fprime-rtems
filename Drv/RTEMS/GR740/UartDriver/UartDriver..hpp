// ======================================================================
// \title  GR740GpioDriver.hpp
// \author Your Name
// \brief  hpp file for GR740GpioDriver component implementation class
//
// \copyright
// Copyright (C) 2023 YourCompany
// ALL RIGHTS RESERVED
//
// ======================================================================

#ifndef DRV_GR740_GPIO_DRIVER_HPP
#define DRV_GR740_GPIO_DRIVER_HPP

#include "Drv/GR740/GPIO/GR740GpioDriverComponentAc.hpp"

namespace Drv {

  class GR740GpioDriver final :
    public GR740GpioDriverComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object GR740GpioDriver
      //!
      GR740GpioDriver(
          const char *const compName /*!< The component name*/
      );

      //! Destroy object GR740GpioDriver
      //!
      ~GR740GpioDriver();

      //! Configure GPIO pin
      //!
      //! \param gpio: GPIO pin number
      //! \param direction: 0 for input, 1 for output
      //! \return Status of configuration
      Drv::GpioStatus configure(
          const U32 gpio, /*!< The GPIO number */
          const U32 direction /*!< 0 for input, 1 for output */
      );

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for gpioRead
      //!
      Drv::GpioStatus gpioRead_handler(
          const FwIndexType portNum, /*!< The port number*/
          Fw::Logic& state
      );

      //! Handler implementation for gpioWrite
      //!
      Drv::GpioStatus gpioWrite_handler(
          const FwIndexType portNum, /*!< The port number*/
          const Fw::Logic& state
      );

    PRIVATE:

      // ----------------------------------------------------------------------
      // Private member variables
      // ----------------------------------------------------------------------

      U32 m_gpio; //!< GPIO pin number
      U32 m_direction; //!< GPIO direction (0=input, 1=output)
  };

} // end namespace Drv

#endif