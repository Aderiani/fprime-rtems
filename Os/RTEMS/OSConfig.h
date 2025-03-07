#ifndef OSCONFIG_H
#define OSCONFIG_H

// RTEMS specific configuration parameters
#define OS_MAX_TASKS          16
#define OS_MAX_QUEUES         16
#define OS_MAX_SEMAPHORES     16
#define OS_MAX_MUTEXES        16
#define OS_STACK_SIZE_MIN     (4 * 1024)  // Minimum stack size in bytes
#define OS_SYSTEM_TIMER_TICKS_PER_SECOND  100  // System tick frequency

// Priority definitions
#define OS_PRIORITY_LOWEST    255
#define OS_PRIORITY_LOW       192
#define OS_PRIORITY_NORMAL    128
#define OS_PRIORITY_HIGH      64
#define OS_PRIORITY_HIGHEST   1

// GR740 specific memory configurations
#define OS_GR740_MEMORY_REGION_SDRAM      0
#define OS_GR740_MEMORY_REGION_SRAM       1

#endif // OSCONFIG_H