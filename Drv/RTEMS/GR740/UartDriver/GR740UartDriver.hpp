// ======================================================================
// \title  UartDriver.hpp
// \author [Your Name]
// \brief  hpp file for UartDriver component implementation class for GR740 (RTEMS)
//
// \copyright
// Copyright 2025, [Your Organization or Name].
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_RTEMS_GR740_UART_DRIVER_HPP
#define DRV_RTEMS_GR740_UART_DRIVER_HPP

#include "Drv/RTEMS/GR740/UartDriver/GR740UartDriverComponentAc.hpp"
#include "Drv/RTEMS/include/DriverCommon.hpp"
#include <Os/Task.hpp>
#include <Fw/Types/SerialBuffer.hpp>

namespace Drv {

class GR740UartDriver final : public GR740UartDriverComponentBase {
  public:
    // UART configuration enums 
    enum UartBaudRate {
        BAUD_9600,
        BAUD_19200,
        BAUD_38400,
        BAUD_57600,
        BAUD_115K,
        BAUD_230K,
        BAUD_460K,
        BAUD_921K,
        BAUD_1000K,
        BAUD_1152K,
        BAUD_1500K,
        BAUD_2000K,
        BAUD_2500K,
        BAUD_3000K,
        BAUD_3500K,
        BAUD_4000K
    };

    enum UartFlowControl {
        NO_FLOW,
        HW_FLOW
    };

    enum UartParity {
        PARITY_NONE,
        PARITY_ODD,
        PARITY_EVEN
    };

    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object GR740UartDriver
    GR740UartDriver(const char* const compName);

    //! Initialize the UART driver
    void init(const NATIVE_INT_TYPE instance);

    //! Open and configure the UART hardware
    bool open(const char* const device, UartBaudRate baud, UartFlowControl fc, UartParity parity, U32 allocationSize);

    //! Destroy object GR740UartDriver
    ~GR740UartDriver();

    //! Start the read task
    void start(Os::Task::ParamType priority, Os::Task::ParamType stackSize, Os::Task::ParamType cpuAffinity);

    //! Quit the read thread
    void quitReadThread();

    //! Join the read task
    Os::Task::Status join();

  PRIVATE:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for send port
    Drv::SendStatus send_handler(
        const NATIVE_INT_TYPE portNum, /*!< The port number */
        Fw::Buffer& serBuffer          /*!< Buffer containing data to send */
    );

    // ----------------------------------------------------------------------
    // Private methods
    // ----------------------------------------------------------------------

    //! Task entry point for reading UART data
    static void serialReadTaskEntry(void* ptr);

    // ----------------------------------------------------------------------
    // Private member variables
    // ----------------------------------------------------------------------

    volatile U32* m_baseAddr;      //!< Pointer to UART register base address
    Fw::String m_device;           //!< Device name (e.g., "UART0")
    U32 m_allocationSize;          //!< Buffer size for receive
    bool m_quitReadThread;         //!< Flag to stop read thread
    Os::Task m_readTask;           //!< Task for reading UART data

    // GR740 APBUART register offsets (based on GRLIB APBUART documentation)
    enum UartRegisters {
        UART_DATA  = 0x00 / 4, //!< Data register
        UART_STAT  = 0x04 / 4, //!< Status register
        UART_CTRL  = 0x08 / 4, //!< Control register
        UART_SCAL  = 0x0C / 4  //!< Scaler (baud rate) register
    };

    // UART control register bits
    enum UartCtrlBits {
        CTRL_RE = 1 << 0,  //!< Receiver enable
        CTRL_TE = 1 << 1,  //!< Transmitter enable
        CTRL_PE = 1 << 2,  //!< Parity enable
        CTRL_PS = 1 << 3,  //!< Parity select (0=even, 1=odd)
        CTRL_FL = 1 << 4   //!< Flow control enable
    };

    // UART status register bits
    enum UartStatBits {
        STAT_DR = 1 << 0, //!< Data ready (receive)
        STAT_TS = 1 << 1, //!< Transmitter shift register empty
        STAT_TH = 1 << 2, //!< Transmitter hold register empty
        STAT_OR = 1 << 3  //!< Overrun error
    };
};

} // end namespace Drv

#endif // DRV_RTEMS_GR740_UART_DRIVER_HPP