#ifndef RTEMS_CONFIG_H
#define RTEMS_CONFIG_H

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/* Use RTEMS Classic API initialization table */
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

/* Define maximum resources */
#define CONFIGURE_MAXIMUM_TASKS 32
#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 32
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_PERIODS 4
#define CONFIGURE_MAXIMUM_REGIONS 2
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 2
#define CONFIGURE_MAXIMUM_TIMERS 8

/* Timing configuration */
#define CONFIGURE_MICROSECONDS_PER_TICK 1000
#define CONFIGURE_TICKS_PER_TIMESLICE 50

/* Filesystem workaround */
#define CONFIGURE_FILESYSTEM_IMFS
#define CONFIGURE_IMFS_DISABLE_CHOWN
#define CONFIGURE_IMFS_DISABLE_CHMOD
#define CONFIGURE_IMFS_DISABLE_UTIME

/* Define struct bintime if missing */
#ifndef _STRUCT_BINTIME
#define _STRUCT_BINTIME
struct bintime {
    time_t sec;    /* Seconds */
    uint64_t frac; /* Fractional part in 2^-64 seconds */
};
#endif

/* Mark as initialization config */
#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#endif