/**
 * \file
 * \author T. Canham
 * \brief Implementation file for channelized telemetry storage component
 *
 * \copyright
 * Copyright 2009-2015, by the California Institute of Technology.
 * ALL RIGHTS RESERVED.  United States Government Sponsorship
 * acknowledged.
 * <br /><br />
 */
#include <FpConfig.hpp>
#include <Fw/Com/ComBuffer.hpp>
#include <Fw/Types/Assert.hpp>
#include <Svc/TlmChan/TlmChan.hpp>

#define DEBUG_TLMCHAN 0  // Set to 0 to disable debug prints

namespace Svc {

TlmChan::TlmChan(const char* name) : TlmChanComponentBase(name), m_activeBuffer(0) {
    // clear slot pointers
    for (NATIVE_UINT_TYPE entry = 0; entry < TLMCHAN_NUM_TLM_HASH_SLOTS; entry++) {
        this->m_tlmEntries[0].slots[entry] = nullptr;
        this->m_tlmEntries[1].slots[entry] = nullptr;
    }
    // clear buckets
    for (NATIVE_UINT_TYPE entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        this->m_tlmEntries[0].buckets[entry].used = false;
        this->m_tlmEntries[0].buckets[entry].updated = false;
        this->m_tlmEntries[0].buckets[entry].bucketNo = entry;
        this->m_tlmEntries[0].buckets[entry].next = nullptr;
        this->m_tlmEntries[0].buckets[entry].id = 0;
        this->m_tlmEntries[1].buckets[entry].used = false;
        this->m_tlmEntries[1].buckets[entry].updated = false;
        this->m_tlmEntries[1].buckets[entry].bucketNo = entry;
        this->m_tlmEntries[1].buckets[entry].next = nullptr;
        this->m_tlmEntries[1].buckets[entry].id = 0;
    }
    // clear free index
    this->m_tlmEntries[0].free = 0;
    this->m_tlmEntries[1].free = 0;
}

TlmChan::~TlmChan() {}

NATIVE_UINT_TYPE TlmChan::doHash(FwChanIdType id) {
    return (id % TLMCHAN_HASH_MOD_VALUE) % TLMCHAN_NUM_TLM_HASH_SLOTS;
}

void TlmChan::pingIn_handler(const FwIndexType portNum, U32 key) {
    // return key
    this->pingOut_out(0, key);
}

void TlmChan::TlmGet_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    // Compute index for entry

    NATIVE_UINT_TYPE index = this->doHash(id);

    // Search to see if channel has been stored
    TlmEntry* entryToUse = this->m_tlmEntries[this->m_activeBuffer].slots[index];
    for (NATIVE_UINT_TYPE bucket = 0; bucket < TLMCHAN_HASH_BUCKETS; bucket++) {
        if (entryToUse) {  // If bucket exists, check id
            if (entryToUse->id == id) {
                break;
            } else {  // otherwise go to next bucket
                entryToUse = entryToUse->next;
            }
        } else {  // no buckets left to search
            break;
        }
    }

    if (entryToUse) {
        val = entryToUse->buffer;
        timeTag = entryToUse->lastUpdate;
    } else {  // requested entry may not be written yet; empty buffer
        val.resetSer();
    }
}

void TlmChan::TlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {

    NATIVE_UINT_TYPE index = this->doHash(id);
    TlmEntry* entryToUse = nullptr;
    TlmEntry* prevEntry = nullptr;

    // Search to see if channel has already been stored or a bucket needs to be added
    if (this->m_tlmEntries[this->m_activeBuffer].slots[index]) {
        entryToUse = this->m_tlmEntries[this->m_activeBuffer].slots[index];
        for (NATIVE_UINT_TYPE bucket = 0; bucket < TLMCHAN_HASH_BUCKETS; bucket++) {
            if (entryToUse) {
                if (entryToUse->id == id) {  // found the matching entry
                    break;
                } else {  // try next entry
                    prevEntry = entryToUse;
                    entryToUse = entryToUse->next;
                }
            } else {
                // Make sure that we haven't run out of buckets
                FW_ASSERT(this->m_tlmEntries[this->m_activeBuffer].free < TLMCHAN_HASH_BUCKETS);
                // add new bucket from free list
                entryToUse =
                    &this->m_tlmEntries[this->m_activeBuffer].buckets[this->m_tlmEntries[this->m_activeBuffer].free++];
                FW_ASSERT(prevEntry);
                prevEntry->next = entryToUse;
                // clear next pointer
                entryToUse->next = nullptr;
                break;
            }
        }
    } else {
        // Make sure that we haven't run out of buckets
        FW_ASSERT(this->m_tlmEntries[this->m_activeBuffer].free < TLMCHAN_HASH_BUCKETS);
        // create new entry at slot head
        this->m_tlmEntries[this->m_activeBuffer].slots[index] =
            &this->m_tlmEntries[this->m_activeBuffer].buckets[this->m_tlmEntries[this->m_activeBuffer].free++];
        entryToUse = this->m_tlmEntries[this->m_activeBuffer].slots[index];
        entryToUse->next = nullptr;
    }

    // copy into entry
    FW_ASSERT(entryToUse);
    entryToUse->used = true;
    entryToUse->id = id;
    entryToUse->updated = true;
    entryToUse->lastUpdate = timeTag;
    entryToUse->buffer = val;
}

void TlmChan::Run_handler(FwIndexType portNum, U32 context) {
    static U32 run_count = 0;

    // Check if PktSend port is connected
    if (not this->isConnected_PktSend_OutputPort(0)) {
        printf("[TLMCHAN] ERROR: PktSend port not connected!\n");
        return;
    }

    // Lock mutex and switch buffers
    this->lock();
    this->m_activeBuffer = 1 - this->m_activeBuffer;

    // Count updated entries
    U32 updatedCount = 0;
    for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        this->m_tlmEntries[this->m_activeBuffer].buckets[entry].updated = false;
        TlmEntry* p_entry = &this->m_tlmEntries[1 - this->m_activeBuffer].buckets[entry];
        if ((p_entry->updated) && (p_entry->used)) {
            updatedCount++;
        }
    }
    this->unLock();

    Fw::TlmPacket pkt;
    pkt.resetPktSer();

    U32 processedCount = 0;
    for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS && processedCount < 3; entry++) {
        TlmEntry* p_entry = &this->m_tlmEntries[1 - this->m_activeBuffer].buckets[entry];
        if ((p_entry->updated) && (p_entry->used)) {
            // printf("[TLMCHAN] Processing entry: id=0x%X\n", p_entry->id);

            Fw::SerializeStatus stat = pkt.addValue(p_entry->id, p_entry->lastUpdate, p_entry->buffer);
            if (stat == Fw::FW_SERIALIZE_OK) {
                processedCount++;
                p_entry->updated = false;
            } else {
                // printf("[TLMCHAN] Failed to add telemetry entry: %d\n", stat);
                break;
            }
        }
    }

    if (pkt.getNumEntries() > 0) {
        // printf("[TLMCHAN] Sending telemetry packet with %llu entries\n", pkt.getNumEntries());
        this->PktSend_out(0, pkt.getBuffer(), 0);
        // printf("[TLMCHAN] Telemetry packet sent successfully\n");
    }
}

// void TlmChan::Run_handler(FwIndexType portNum, U32 context) {
//     static U32 run_count = 0;
//     printf("[TLMCHAN] Run handler called: count=%u, portNum=%u, context=%u\n", ++run_count, portNum, context);

//     // Only write packets if connected
//     if (not this->isConnected_PktSend_OutputPort(0)) {
//         printf("[TLMCHAN] PktSend port not connected\n");
//         return;
//     }

//     printf("[TLMCHAN] PktSend port is connected, proceeding with telemetry processing\n");

//     // lock mutex long enough to modify active telemetry buffer
//     // so the data can be read without worrying about updates
//     this->lock();
//     this->m_activeBuffer = 1 - this->m_activeBuffer;
//     // set activeBuffer to not updated
//     for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
//         this->m_tlmEntries[this->m_activeBuffer].buckets[entry].updated = false;
//     }
//     this->unLock();

//     // Count updated entries for debugging
//     U32 updatedCount = 0;
//     for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
//         TlmEntry* p_entry = &this->m_tlmEntries[1 - this->m_activeBuffer].buckets[entry];
//         if ((p_entry->updated) && (p_entry->used)) {
//             updatedCount++;
//         }
//     }
//     printf("[TLMCHAN] Found %u updated telemetry entries to process\n", updatedCount);

//     // go through each entry and send a packet if it has been updated
//     Fw::TlmPacket pkt;
//     pkt.resetPktSer();
//     U32 entriesInPacket = 0;

//     for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
//         TlmEntry* p_entry = &this->m_tlmEntries[1 - this->m_activeBuffer].buckets[entry];
//         if ((p_entry->updated) && (p_entry->used)) {
//             printf("[TLMCHAN] Processing updated entry: id=0x%X, size=%llu\n",
//                    p_entry->id, p_entry->buffer.getBuffLength());

//             Fw::SerializeStatus stat = pkt.addValue(p_entry->id, p_entry->lastUpdate, p_entry->buffer);

//             // check to see if this packet is full, if so, send it
//             if (Fw::FW_SERIALIZE_NO_ROOM_LEFT == stat) {
//                 printf("[TLMCHAN] Packet full, sending packet with %llu entries to PktSend_out\n",
//                 pkt.getNumEntries());
//                 // Hexdump first few bytes of buffer for debugging
//                 printf("[TLMCHAN] Packet data: ");
//                 const U8* bufAddr = pkt.getBuffer().getBuffAddr();
//                 U32 bufLen = pkt.getBuffer().getBuffLength();
//                 for (U32 i = 0; i < (bufLen > 16 ? 16 : bufLen); i++) {
//                     printf("%02X ", bufAddr[i]);
//                 }
//                 printf("\n");

//                 this->PktSend_out(0, pkt.getBuffer(), 0);
//                 entriesInPacket = 0;
//                 // reset packet for more entries
//                 pkt.resetPktSer();
//                 // add entry to new packet
//                 stat = pkt.addValue(p_entry->id, p_entry->lastUpdate, p_entry->buffer);
//                 // if this doesn't work, that means packet isn't big enough for
//                 // even one channel, so assert
//                 FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<FwAssertArgType>(stat));
//                 entriesInPacket++;
//             } else if (Fw::FW_SERIALIZE_OK == stat) {
//                 entriesInPacket++;
//                 // if there was still room, do nothing move on to the next channel in the packet
//             } else  // any other status is an assert, since it shouldn't happen
//             {
//                 FW_ASSERT(0, static_cast<FwAssertArgType>(stat));
//             }
//             // flag as updated
//             p_entry->updated = false;
//         }  // end if entry was updated
//     }  // end for each entry

//     // send remnant entries
//     if (pkt.getNumEntries() > 0) {
//         printf("[TLMCHAN] Sending final packet with %llu entries to PktSend_out\n", pkt.getNumEntries());
//         // Hexdump first few bytes of buffer for debugging
//         printf("[TLMCHAN] Final packet data: ");
//         const U8* bufAddr = pkt.getBuffer().getBuffAddr();
//         U32 bufLen = pkt.getBuffer().getBuffLength();
//         for (U32 i = 0; i < (bufLen > 16 ? 16 : bufLen); i++) {
//             printf("%02X ", bufAddr[i]);
//         }
//         printf("\n");

//         this->PktSend_out(0, pkt.getBuffer(), 0);
//     } else {
//         printf("[TLMCHAN] No telemetry entries to send\n");
//     }

//     printf("[TLMCHAN] Run handler completed - sent packets with total %u updated entries\n", updatedCount);
// }  // end run handler

}  // namespace Svc
