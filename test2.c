#include <rtems.h>
#include <stdio.h>

// Define test_task at global scope
rtems_task test_task(rtems_task_argument arg)
{
    printf("Test task started\n");
    while (1) {
        printf("Test task heartbeat\n");
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 2);
    }
}

rtems_task Init(rtems_task_argument ignored)
{
    printf("RTEMS Test Application Started\n");
    
    rtems_id task_id;
    rtems_status_code status;
    
    status = rtems_task_create(
        rtems_build_name('T', 'E', 'S', 'T'),
        100,
        8 * 1024,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &task_id
    );
    
    if (status != RTEMS_SUCCESSFUL) {
        printf("Failed to create test task: %d\n", status);
        rtems_task_suspend(RTEMS_SELF);
    }
    
    status = rtems_task_start(task_id, test_task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        printf("Failed to start test task: %d\n", status);
    } else {
        printf("Test task started successfully\n");
    }
    
    while (1) {
        rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 5);
        printf("Init task still running\n");
    }
}

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS 2
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>