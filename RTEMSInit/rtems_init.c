
#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE  // For compatibility with older versions of RTEMS
#endif

#include "rtems_config.h"
#include <drvmgr/drvmgr_confdefs.h>
#include <rtems.h>
#include <rtems/confdefs.h>
#include <stddef.h>
#include <grlib/network_interface_add.h>  // This file defines the ethernet_config struct
#include <grlib/ambapp_bus.h>
#include <rtems/rtems/clock.h>
#include <time.h>


void initialize_rtems_clock(void) {
    // Check current time first
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    // printf("RTEMS clock BEFORE set: %ld.%09ld\n", current_time.tv_sec, current_time.tv_nsec);
    
    // Method 1: Use RTEMS Time of Day API
    rtems_time_of_day tod;
    
    // Set to Jan 1, 2025, 00:00:00
    tod.year   = 2025;
    tod.month  = 6;
    tod.day    = 12;  // Adjusted to June 12, 2025
    tod.hour   = 15;
    tod.minute = 38;
    tod.second = 0;
    tod.ticks  = 0;
    
    // printf("Setting RTEMS TOD to: %d-%02d-%02d %02d:%02d:%02d\n",
    //        tod.year, tod.month, tod.day, tod.hour, tod.minute, tod.second);
    
    rtems_status_code status = rtems_clock_set(&tod);
    
    if (status != RTEMS_SUCCESSFUL) {
        printf("ERROR: rtems_clock_set failed with status: %d\n", status);
        
        // Method 2: Try using timespec directly with RTEMS internal API
        struct timespec ts;
        ts.tv_sec = 1704067200;  // Jan 1, 2024 in epoch seconds
        ts.tv_nsec = 0;
        
        // Use RTEMS internal function if available
        #ifdef _RTEMS_SCORE_TODIMPL_H
        _TOD_Set(&ts);
        printf("Used _TOD_Set to set time\n");
        #else
        printf("WARNING: Could not set RTEMS clock - no alternative method available\n");
        #endif
    } else {
        // printf("rtems_clock_set succeeded\n");
    }
    
    // Verify the time was set
    clock_gettime(CLOCK_REALTIME, &current_time);
    printf("RTEMS clock AFTER set: %ld.%09ld\n", current_time.tv_sec, current_time.tv_nsec);
    
    // If still in 1988, warn the user
    if (current_time.tv_sec < 1000000000) {
        printf("WARNING: RTEMS clock is still in 1988 epoch!\n");
        printf("This RTEMS configuration may not support clock setting.\n");
    }
}

struct ethernet_config interface_configs[] = {
    { "192.168.0.67", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x79}},
    { NULL, NULL, {0,0,0,0,0,0}}
};

int main(int argc, char* argv[]) {
    return fprime_main(argc, argv);
}