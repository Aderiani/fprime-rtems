#include "Svc/ChronoTime/ChronoTime.hpp"
#include <chrono>
#include <time.h>  // Add this for clock_gettime
#include <stdio.h> // Add for debugging
#include "FpConfig.hpp"

namespace Svc {

ChronoTime::ChronoTime(const char* const compName) : ChronoTimeComponentBase(compName) {}

ChronoTime::~ChronoTime() {}

void ChronoTime::timeGetPort_handler(FwIndexType portNum, Fw::Time& time) {
    // Option 1: Use std::chrono (current implementation)
    const auto time_now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_now.time_since_epoch()).count();
    const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch()).count() % 1000000;
    
    // Debug: Check if std::chrono is giving bad time
    if (seconds < 1000000000) {
        printf("[ChronoTime] WARNING: std::chrono giving 1988 time: %ld seconds\n", (long)seconds);
        
        // Option 2: Fallback to POSIX clock_gettime for RTEMS
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
            time.set(TimeBase::TB_WORKSTATION_TIME,
                     0,  // context
                     static_cast<U32>(ts.tv_sec),
                     static_cast<U32>(ts.tv_nsec / 1000));  // Convert nanoseconds to microseconds
            
            printf("[ChronoTime] Using POSIX time instead: %u.%06u\n", 
                   (unsigned)ts.tv_sec, (unsigned)(ts.tv_nsec/1000));
            return;
        }
    }
    
    // Use the std::chrono time if it looks valid
    time.set(TimeBase::TB_WORKSTATION_TIME,
             0,  // context  
             static_cast<U32>(seconds),
             static_cast<U32>(microseconds));
}

}  // namespace Svc