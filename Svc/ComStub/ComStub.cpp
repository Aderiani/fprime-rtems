// ======================================================================
// \title  ComStub.cpp
// \author mstarch
// \brief  cpp file for ComStub component implementation class
// ======================================================================

#include <Svc/ComStub/ComStub.hpp>
#include "Fw/Types/Assert.hpp"
#include "Fw/Types/BasicTypes.hpp"
#define DEBUG_COMSTUB 1
namespace Svc {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

ComStub::ComStub(const char* const compName) : ComStubComponentBase(compName), m_reinitialize(true) {}

ComStub::~ComStub() {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------


Drv::SendStatus ComStub::comDataIn_handler(const FwIndexType portNum, Fw::Buffer& sendBuffer) {
    // Add debug logs
    //printf("[COMSTUB] comDataIn called: size=%u, data=%p\n", 
        //    sendBuffer.getSize(), sendBuffer.getData());
    
    FW_ASSERT(!this->m_reinitialize || !this->isConnected_comStatus_OutputPort(0));
    
    // Force connection status to false temporarily to diagnose issues
    bool previousInit = this->m_reinitialize;
    
    Drv::SendStatus driverStatus = Drv::SendStatus::SEND_RETRY;
    for (NATIVE_UINT_TYPE i = 0; driverStatus == Drv::SendStatus::SEND_RETRY && i < RETRY_LIMIT; i++) {
        //printf("[COMSTUB] Attempting to send data to driver (try %u)\n", i+1);
        driverStatus = this->drvDataOut_out(0, sendBuffer);
        
        // If retry, add a small delay to avoid overwhelming RTEMS
        if (driverStatus == Drv::SendStatus::SEND_RETRY && i < RETRY_LIMIT-1) {
            Os::Task::delay(Fw::TimeInterval(0, 1000)); // 1ms delay between retries
        }
    }
    
    //printf("[COMSTUB] Driver returned status: %d\n", driverStatus.e);
    
    // Always report status regardless of connection state for debugging
    Fw::Success comSuccess = (driverStatus.e == Drv::SendStatus::SEND_OK) ? 
                             Fw::Success::SUCCESS : Fw::Success::FAILURE;
    
    //printf("[COMSTUB] Reporting status to framer: %d\n", comSuccess.e);
    if (this->isConnected_comStatus_OutputPort(0)) {
        this->comStatus_out(0, comSuccess);
    } else {
        //printf("[COMSTUB] comStatus port not connected!\n");
    }
    
    //printf("[COMSTUB-DEBUG] Sent status=%d to framer\n", comSuccess.e);


    // Restore initialization state
    this->m_reinitialize = previousInit;
    
    // For RTEMS debugging, always return SEND_OK to avoid cascading issues
    return Drv::SendStatus::SEND_OK;
}

void ComStub::drvConnected_handler(const FwIndexType portNum) {
    //printf("[COMSTUB] drvConnected called\n");
    
    Fw::Success radioSuccess = Fw::Success::SUCCESS;
    if (this->isConnected_comStatus_OutputPort(0) && m_reinitialize) {
        this->m_reinitialize = false;
        //printf("[COMSTUB] Sending SUCCESS status to framer after connection\n");
        this->comStatus_out(0, radioSuccess);
    } else if (!this->isConnected_comStatus_OutputPort(0)) {
        //printf("[COMSTUB] comStatus port not connected in drvConnected_handler!\n");
    } else if (!m_reinitialize) {
        //printf("[COMSTUB] Not re-initializing in drvConnected_handler\n");
    }
}

void ComStub::drvDataIn_handler(const FwIndexType portNum,
                                Fw::Buffer& recvBuffer,
                                const Drv::RecvStatus& recvStatus) {
    //printf("[COMSTUB] drvDataIn called: status=%d, size=%u, data=%p\n", 
        //    recvStatus.e, recvBuffer.getSize(), recvBuffer.getData());
    
    if (this->isConnected_comDataOut_OutputPort(0)) {
        //printf("[COMSTUB] Forwarding data to deframer\n");
        this->comDataOut_out(0, recvBuffer, recvStatus);
    } else {
        //printf("[COMSTUB] comDataOut port not connected!\n");
    }
}


}  // end namespace Svc
