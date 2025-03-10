// ======================================================================
// \title  UartDriver.hpp
// \author fprime-community
// \brief  hpp file for GR740UartDriver component implementation class
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_GR740_UART_DRIVER_HPP
#define DRV_GR740_UART_DRIVER_HPP

#include <Drv/RTEMS/GR740/UartDriver/UartDriverComponentAc.hpp>
#include <Drv/RTEMS/include/DriverCommon.hpp>
#include <Os/Mutex.hpp>
#include <Os/Task.hpp>

// GRLIB APBUART driver includes
#include <grlib/apbuart.h>

// Buffer size for receiving data
#define UART_RECEIVE_BUFFER_SIZE 256

namespace Drv {

  class GR740UartDriver final : public GR740UartDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object GR740UartDriver
    //!
    GR740UartDriver(
        const char* const compName, /*!< The component name*/
        const U32 uartInstance      /*!< The UART instance number to use*/
    );

    //! Initialize component
    //!
    void init(
        const FwIndexType instance = 0 /*!< The instance number*/
    );

    //! Destroy object GR740UartDriver
    //!
    ~GR740UartDriver();

    // ----------------------------------------------------------------------
    // UART configuration types
    // ----------------------------------------------------------------------
    
    struct UartConfiguration {
      U32 baudRate;             //!< Baud rate (bps)
      U8 dataBits;              //!< Data bits (5-8)
      U8 stopBits;              //!< Stop bits (1-2)
      bool parityEnabled;       //!< Parity enabled flag
      bool parityOdd;           //!< Parity odd (true) or even (false)
      bool flowControlEnabled;  //!< Hardware flow control enabled flag
    };

    //! \brief Initialize the UART driver
    //!
    //! This function initializes the UART driver by finding the APBUART device
    //! in the system and setting up the driver.
    //!
    //! \return true if initialization was successful, false otherwise
    bool initialize();

    //! \brief Configure the UART
    //!
    //! \param config UART configuration
    //! \return true if configuration was successful, false otherwise
    bool configure(const UartConfiguration& config);

    //! \brief Start the receive task
    //!
    //! \param priority Task priority (default: OS task default)
    //! \param stackSize Task stack size (default: OS task default)
    //! \return true if task started successfully, false otherwise
    bool startReceiveTask(
      NATIVE_INT_TYPE priority = Os::Task::TASK_DEFAULT,
      NATIVE_INT_TYPE stackSize = Os::Task::TASK_DEFAULT);

    //! \brief Stop the receive task
    void stopReceiveTask();

  PRIVATE:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for drvDataIn
    //!
    Drv::SendStatus drvDataIn_handler(
        const FwIndexType portNum, /*!< The port number*/
        Fw::Buffer &fwBuffer
    ) override;

    //! \brief Receive task entry point
    //!
    //! \param arg Pointer to driver instance
    static void receiveTaskEntry(void* arg);

    //! \brief Receive task implementation
    void receiveTask();

    //! \brief Send a buffer over UART
    //!
    //! \param buffer Data buffer to send
    //! \param size Size of data to send
    //! \return true if send was successful, false otherwise
    bool sendBuffer(const U8* buffer, size_t size);

    //! \brief Receive data from UART
    //!
    //! \param buffer Buffer to store received data
    //! \param size Size of buffer
    //! \return Number of bytes received, negative value on error
    I32 receiveBuffer(U8* buffer, size_t size);

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------
    const U32 m_uartInstance;                 //!< UART instance number
    struct drvmgr_dev* m_uartDevice;          //!< APBUART device pointer
    struct apbuart_regs* m_uartRegs;          //!< APBUART register structure
    bool m_initialized;                        //!< Initialization flag
    bool m_configured;                         //!< Configuration flag
    UartConfiguration m_config;                //!< Current configuration
    Os::Task m_receiveTask;                    //!< Receive task
    Os::Mutex m_mutex;                         //!< Mutex for thread protection
    bool m_receiveTaskRunning;                 //!< Flag to control receive task
    U8 m_receiveBuffer[UART_RECEIVE_BUFFER_SIZE]; //!< Buffer for received data
  };

} // end namespace Drv

#endif