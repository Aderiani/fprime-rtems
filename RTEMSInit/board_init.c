
// RTEMSInit/board_init.c


#include <rtems.h>
#include <stdio.h>
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp_bus.h>
#include <grlib/ambapp_ids.h>
// Forward declarations
extern void system_init2(void);

// In board_init.c
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp_bus.h>
#include <grlib/ambapp_ids.h>

// Import the driver resources structure
extern struct drvmgr_bus_res grlib_drv_resources;

int board_initialize(void) {
    printf("*** GR740 Board Initialization ***\n");
    
    // Initialize driver manager
    printf("Initializing driver manager\n");
    int status = drvmgr_init();
    if (status != 0) {
        printf("Driver manager failed to initialize: %d\n", status);
        return -1;
    }
    
    printf("Driver manager initialized, registering GRLIB devices...\n");
    
    // Register GRLIB root bus manually (this is what system_init2 would do)
    #ifndef RTEMS_DRVMGR_STARTUP
    struct grlib_config grlib_bus_config = {
        &ambapp_plb,             /* AMBAPP bus setup */
        &grlib_drv_resources,    /* Driver configuration */
    };
    
    // Register GRLIB root bus
    printf("Manually registering GRLIB root bus\n");
    ambapp_grlib_root_register(&grlib_bus_config);
    #else
    printf("Using RTEMS_DRVMGR_STARTUP, driver resources defined but registration handled by BSP\n");
    #endif
    
    // Print registered devices 
    printf("Checking available drivers:\n");
    drvmgr_info_drvs(0);
    
    printf("Board initialization complete\n");
    return 0;
}