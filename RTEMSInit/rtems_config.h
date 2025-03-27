#ifndef RTEMS_CONFIG_H
#define RTEMS_CONFIG_H

// Required drivers
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

// Init task table required
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

// System resources
#define CONFIGURE_MAXIMUM_TASKS 32
#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 32
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

// Required for networking
#define CONFIGURE_MAXIMUM_DRIVERS 32

// Use unlimited objects where possible
#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

// Include IMFS filesystem
#define CONFIGURE_FILESYSTEM_IMFS

// Define struct bintime if missing (add this part)
#include <stdint.h>
#include <time.h>
#ifndef _STRUCT_BINTIME
#define _STRUCT_BINTIME
struct bintime {
    time_t sec;    /* Seconds */
    uint64_t frac; /* Fractional part in 2^-64 seconds */
};
#endif

// Initialization
#define CONFIGURE_INIT

// Include the actual configuration
#include <rtems/confdefs.h>

#endif // RTEMS_CONFIG_H