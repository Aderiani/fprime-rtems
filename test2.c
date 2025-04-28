#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>
// Define the Init task
rtems_task Init(rtems_task_argument ignored) {
    unsigned int counter = 0;
    rtems_interval ticks_per_second;

    printf(" GR740 RTEMS Test Application Starting \n");

    // Get ticks per second
    ticks_per_second = rtems_clock_get_ticks_per_second();
    printf("System clock: %u ticks per second\n", ticks_per_second);

    // Print output periodically to show we're alive
    printf("Entering main loop...\n");
    while (1) {
        printf("Test application alive - count: %u\n", counter++);

        // Sleep for 1 second
        rtems_task_wake_after(ticks_per_second);
    }

    // This will never be reached
    rtems_task_delete(RTEMS_SELF);
}
// Define configuration
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS 4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>