#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>
#include <cstdio>
#include <cstring>

namespace Svc {

RateGroupDriver::RateGroupDriver(const char* compName)
    : RateGroupDriverComponentBase(compName), m_ticks(0), m_rollover(1), m_configured(false) {}

void RateGroupDriver::configure(const DividerSet& dividerSet) {
    //printf("RateGroupDriver: Configuring with %lu dividers\n",
        //    (unsigned long)FW_NUM_ARRAY_ELEMENTS(dividerSet.dividers));

    for (NATIVE_INT_TYPE i = 0; i < static_cast<NATIVE_INT_TYPE>(FW_NUM_ARRAY_ELEMENTS(dividerSet.dividers)); i++) {
        //printf("RateGroupDriver: Divider[%d] = {div:%d, offset:%d}\n", i, dividerSet.dividers[i].divisor,
            //    dividerSet.dividers[i].offset);
    }

    // check arguments
    FW_ASSERT(dividerSet.dividers);
    // verify port/table size matches
    FW_ASSERT(FW_NUM_ARRAY_ELEMENTS(this->m_dividers) == this->getNum_CycleOut_OutputPorts(),
              static_cast<NATIVE_INT_TYPE>(FW_NUM_ARRAY_ELEMENTS(this->m_dividers)),
              this->getNum_CycleOut_OutputPorts());
    // copy provided array of dividers
    for (NATIVE_UINT_TYPE entry = 0; entry < RateGroupDriver::DIVIDER_SIZE; entry++) {
        // A port with an offset equal or bigger than the divisor is not accepted because it would never be called
        FW_ASSERT((dividerSet.dividers[entry].offset == 0) ||
                      (dividerSet.dividers[entry].offset < dividerSet.dividers[entry].divisor),
                  dividerSet.dividers[entry].offset, dividerSet.dividers[entry].divisor);
        this->m_dividers[entry] = dividerSet.dividers[entry];
        // rollover value should be product of all dividers to make sure integer rollover doesn't jump cycles
        // only use non-zero dividers
        if (dividerSet.dividers[entry].divisor != 0) {
            this->m_rollover *= dividerSet.dividers[entry].divisor;
        }
    }
    this->m_configured = true;
}

RateGroupDriver::~RateGroupDriver() {}

void RateGroupDriver::CycleIn_handler(FwIndexType portNum, Os::RawTime& cycleStart) {
    //printf("[RG-DRIVER] CycleIn called, portNum=%d, tick=%u\n", portNum, this->m_ticks);

    // For each divider entry
    for (NATIVE_INT_TYPE entry = 0; entry < static_cast<NATIVE_INT_TYPE>(FW_NUM_ARRAY_ELEMENTS(this->m_dividers));
         entry++) {
        if (this->m_dividers[entry].divisor != 0) {
            bool outputTriggered =
                ((this->m_ticks % this->m_dividers[entry].divisor) == this->m_dividers[entry].offset);

            if (outputTriggered) {
                if (this->isConnected_CycleOut_OutputPort(entry)) {
                    //printf("[RG-DRIVER] Triggering CycleOut port %d\n", entry);
                    this->CycleOut_out(entry, cycleStart);
                } else {
                    //printf("[RG-DRIVER] Port %d not connected\n", entry);
                }
            }
        }
    }

    // Rollover the tick value when the tick count reaches the rollover value
    this->m_ticks = (this->m_ticks + 1) % this->m_rollover;
    //printf("RateGroupDriver: CycleIn_handler completed\n");
}

}  // namespace Svc
