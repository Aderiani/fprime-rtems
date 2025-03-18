// ======================================================================
// \title  UartDriver.cpp
// \author [Your Name]
// \brief  cpp file for UartDriver component implementation class for GR740 (RTEMS)
//
// \copyright
// Copyright 2025, [Your Organization or Name].
// ALL RIGHTS RESERVED.
//
// ======================================================================

#include <Drv/RTEMS/GR740/UartDriver/UartDriver.hpp>
#include <Fw/Types/Assert.hpp>
#include "Fw/Types/BasicTypes.hpp"
namespace Drv {

UartDriver::UartDriver(const char* const compName)
    : GR740UartDriverComponentBase(compName),
      m_baseAddr(nullptr),
      m_device("NOT_EXIST"),
      m_allocationSize(0),
      m_quitReadThread(false) {}

void UartDriver::init(const NATIVE_INT_TYPE instance) {
    GR740UartDriverComponentBase::init(instance);
}

bool UartDriver::open(const char* const device, UartBaudRate baud, UartFlowControl fc, UartParity parity, U32 allocationSize) {
    FW_ASSERT(device != nullptr);
    m_allocationSize = allocationSize;
    m_device = device;

    // Map device name to GR740 UART base address
    if (strcmp(device, "UART0") == 0) {
        m_baseAddr = reinterpret_cast<volatile U32*>(RTEMS::BaseAddress::UART0);
    } else if (strcmp(device, "UART1") == 0) {
        m_baseAddr = reinterpret_cast<volatile U32*>(RTEMS::BaseAddress::UART1);
    } else {
        this->log_WARNING_HI_OpenError(Fw::LogStringArg(device), -1, Fw::LogStringArg("Unknown device"));
        return false;
    }

    if (!m_baseAddr) {
        this->log_WARNING_HI_OpenError(Fw::LogStringArg(device), -1, Fw::LogStringArg("Invalid base address"));
        return false;
    }

    // Calculate scaler for baud rate (assuming 50 MHz system clock)
    U32 scaler = 0;
    switch (baud) {
        case BAUD_9600:   scaler = (50000000 / (9600 * 8)) - 1; break;
        case BAUD_19200:  scaler = (50000000 / (19200 * 8)) - 1; break;
        case BAUD_38400:  scaler = (50000000 / (38400 * 8)) - 1; break;
        case BAUD_57600:  scaler = (50000000 / (57600 * 8)) - 1; break;
        case BAUD_115K:   scaler = (50000000 / (115200 * 8)) - 1; break;
        case BAUD_230K:   scaler = (50000000 / (230400 * 8)) - 1; break;
        case BAUD_460K:   scaler = (50000000 / (460800 * 8)) - 1; break;
        case BAUD_921K:   scaler = (50000000 / (921600 * 8)) - 1; break;
        case BAUD_1000K:  scaler = (50000000 / (1000000 * 8)) - 1; break;
        case BAUD_1152K:  scaler = (50000000 / (1152000 * 8)) - 1; break;
        case BAUD_1500K:  scaler = (50000000 / (1500000 * 8)) - 1; break;
        case BAUD_2000K:  scaler = (50000000 / (2000000 * 8)) - 1; break;
        case BAUD_2500K:  scaler = (50000000 / (2500000 * 8)) - 1; break;
        case BAUD_3000K:  scaler = (50000000 / (3000000 * 8)) - 1; break;
        case BAUD_3500K:  scaler = (50000000 / (3500000 * 8)) - 1; break;
        case BAUD_4000K:  scaler = (50000000 / (4000000 * 8)) - 1; break;
        default:
            FW_ASSERT(0, static_cast<FwAssertArgType>(baud));
            break;
    }

    // Configure UART
    U32 ctrl = CTRL_RE | CTRL_TE; // Enable receiver and transmitter
    if (fc == HW_FLOW) {
        ctrl |= CTRL_FL;
    }
    if (parity == PARITY_EVEN) {
        ctrl |= CTRL_PE;
    } else if (parity == PARITY_ODD) {
        ctrl |= CTRL_PE | CTRL_PS;
    }

    m_baseAddr[UART_SCAL] = scaler;
    m_baseAddr[UART_CTRL] = ctrl;

    this->log_ACTIVITY_HI_PortOpened(Fw::LogStringArg(device));
    if (this->isConnected_ready_OutputPort(0)) {
        this->ready_out(0); // Indicate the driver is connected
    }
    return true;
}

UartDriver::~UartDriver() {
    if (m_baseAddr) {
        m_baseAddr[UART_CTRL] = 0; // Disable UART
    }
}

// ----------------------------------------------------------------------
// Handler implementations
// ----------------------------------------------------------------------

Drv::SendStatus UartDriver::send_handler(const NATIVE_INT_TYPE portNum, Fw::Buffer& serBuffer) {
    Drv::SendStatus status = Drv::SendStatus::SEND_OK;
    if (!m_baseAddr || serBuffer.getData() == nullptr || serBuffer.getSize() == 0) {
        status = Drv::SendStatus::SEND_ERROR;
    } else {
        U8* data = serBuffer.getData();
        U32 size = serBuffer.getSize();

        for (U32 i = 0; i < size; i++) {
            // Wait for transmitter to be ready
            while (!(m_baseAddr[UART_STAT] & STAT_TH)) {
                // Check for overrun or other errors
                if (m_baseAddr[UART_STAT] & STAT_OR) {
                    this->log_WARNING_HI_WriteError(Fw::LogStringArg(m_device), -1);
                    status = Drv::SendStatus::SEND_ERROR;
                    break;
                }
            }
            if (status == Drv::SendStatus::SEND_ERROR) {
                break;
            }
            m_baseAddr[UART_DATA] = data[i];
        }
    }

    if (isConnected_deallocate_OutputPort(0)) {
        deallocate_out(0, serBuffer);
    }
    return status;
}

// ----------------------------------------------------------------------
// Private methods
// ----------------------------------------------------------------------

void UartDriver::serialReadTaskEntry(void* ptr) {
    FW_ASSERT(ptr != nullptr);
    UartDriver* comp = reinterpret_cast<UartDriver*>(ptr);

    while (!comp->m_quitReadThread) {
        Fw::Buffer buff = comp->allocate_out(0, comp->m_allocationSize);

        if (buff.getData() == nullptr) {
            comp->log_WARNING_HI_NoBuffers(Fw::LogStringArg(comp->m_device));
            comp->recv_out(0, buff, Drv::RecvStatus::RECV_ERROR);
            Os::Task::delay(Fw::TimeInterval(0, 50000)); // 50 ms delay
            continue;
        }

        U8* data = buff.getData();
        U32 size = buff.getSize();
        U32 bytesRead = 0;

        // Read with timeout (1 sec equivalent)
        U32 timeout = 10000; // ~1 sec at 1 kHz tick rate, adjust as needed
        while (bytesRead < size && timeout-- && !comp->m_quitReadThread) {
            if (comp->m_baseAddr[UART_STAT] & STAT_DR) { // Data ready
                data[bytesRead++] = static_cast<U8>(comp->m_baseAddr[UART_DATA]);
            }
            if (comp->m_baseAddr[UART_STAT] & STAT_OR) { // Overrun error
                comp->log_WARNING_HI_ReadError(Fw::LogStringArg(comp->m_device), -1);
                break;
            }
        }

        Drv::RecvStatus status = Drv::RecvStatus::RECV_OK;
        if (bytesRead > 0) {
            buff.setSize(bytesRead);
        } else {
            status = Drv::RecvStatus::RECV_ERROR;
        }
        comp->recv_out(0, buff, status);
    }
}

void UartDriver::start(Os::Task::ParamType priority, Os::Task::ParamType stackSize, Os::Task::ParamType cpuAffinity) {
    Os::TaskString task("UartReader");
    Os::Task::Arguments arguments(task, serialReadTaskEntry, this, priority, stackSize, cpuAffinity);
    Os::Task::Status stat = this->m_readTask.start(arguments);
    FW_ASSERT(stat == Os::Task::OP_OK, stat);
}

void UartDriver::quitReadThread() {
    this->m_quitReadThread = true;
}

Os::Task::Status UartDriver::join() {
    return m_readTask.join();
}

} // end namespace Drv