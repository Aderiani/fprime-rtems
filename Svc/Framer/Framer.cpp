// ======================================================================
// \title  Framer.cpp
// \author mstarch
// \brief  cpp file for Framer component implementation class
//
// \copyright
// Copyright 2009-2022, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#include <FpConfig.hpp>
#include <Svc/Framer/Framer.hpp>
#include "Fw/Logger/Logger.hpp"
#include "Utils/Hash/Hash.hpp"

#define DEBUG_FRAMER 1  // Set to 0 to disable debug prints


namespace Svc {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

Framer ::Framer(const char* const compName)
    : FramerComponentBase(compName), FramingProtocolInterface(), m_protocol(nullptr), m_frame_sent(false) {}

Framer ::~Framer() {}

void Framer ::setup(FramingProtocol& protocol) {
    FW_ASSERT(this->m_protocol == nullptr);
    this->m_protocol = &protocol;
    protocol.setup(*this);
}

void Framer ::handle_framing(const U8* const data, const U32 size, Fw::ComPacket::ComPacketType packet_type) {

    #if DEBUG_FRAMER
    printf("[FRAMER] Handling frame: size=%u, type=%d\n", size, packet_type);
    #endif

    FW_ASSERT(this->m_protocol != nullptr);
    this->m_frame_sent = false;  // Clear the flag to detect if frame was sent
    this->m_protocol->frame(data, size, packet_type);
    // If no frame was sent, Framer has the obligation to report success
    if (this->isConnected_comStatusOut_OutputPort(0) && (!this->m_frame_sent)) {
        Fw::Success status = Fw::Success::SUCCESS;
        this->comStatusOut_out(0, status);
    }
}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void Framer ::comIn_handler(const FwIndexType portNum, Fw::ComBuffer& data, U32 context) {
    FW_ASSERT(data.getBuffLength() < std::numeric_limits<U32>::max(), static_cast<FwAssertArgType>(data.getBuffLength()));
    this->handle_framing(data.getBuffAddr(), static_cast<U32>(data.getBuffLength()), Fw::ComPacket::FW_PACKET_UNKNOWN);
}

void Framer ::bufferIn_handler(const FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->handle_framing(fwBuffer.getData(), fwBuffer.getSize(), Fw::ComPacket::FW_PACKET_FILE);
    // Deallocate the buffer after it was processed by the framing protocol
    this->bufferDeallocate_out(0, fwBuffer);
}

void Framer ::comStatusIn_handler(const FwIndexType portNum, Fw::Success& condition) {
    if (this->isConnected_comStatusOut_OutputPort(portNum)) {
        this->comStatusOut_out(portNum, condition);
    }
}

// ----------------------------------------------------------------------
// Framing protocol implementations
// ----------------------------------------------------------------------

void Framer ::send(Fw::Buffer& outgoing) {

    #if DEBUG_FRAMER
    printf("[FRAMER] Sending packet: size=%u, data=%p\n", 
           outgoing.getSize(), outgoing.getData());
    
    // Print packet type
    // if (outgoing.getSize() >= 4) {
    //     FwPacketDescriptorType descriptor;
    //     Fw::SerializeBufferBase sb(outgoing.getData(), 4);
    //     sb.deserialize(descriptor);
    //     printf("[FRAMER] Packet type: 0x%X\n", descriptor);
    // }
    #endif


    FW_ASSERT(!this->m_frame_sent); // Prevent multiple sends per-packet
    const Drv::SendStatus sendStatus = this->framedOut_out(0, outgoing);
    if (sendStatus.e != Drv::SendStatus::SEND_OK) {
        // Note: if there is a data sending problem, an EVR likely wouldn't
        // make it down. Log the issue in hopes that
        // someone will see it.
        Fw::Logger::log("[ERROR] Failed to send framed data: %d\n", sendStatus.e);
    }
    this->m_frame_sent = true;  // A frame was sent
}

Fw::Buffer Framer ::allocate(const U32 size) {
    return this->framedAllocate_out(0, size);
}

}  // end namespace Svc
