
// RTEMSInit/board_init.c
#include <stdio.h>
#include <rtems.h>

// Forward declarations
extern void system_init2(void);
extern struct drvmgr_bus_res grlib_drv_resources;

// Make sure this runs before any network initialization
int board_initialize(void) {
    printf("*** GR740 Board Initialization ***\n");
    
    // Initialize driver manager with our resources
    printf("Initializing driver manager\n");
    int status = drvmgr_init();
    if (status != 0) {
        printf("Driver manager failed to initialize: %d\n", status);
        return -1;
    }
    
    // Initialize GRLIB-specific hardware
    printf("Initializing GRLIB hardware\n");
    system_init2();
    
    // Print confirmation
    printf("Board initialization complete\n");
    return 0;
}