// test_fprime_style.c
#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>
#include <drvmgr/drvmgr.h>

// Forward declaration for a function that would call the F' initialization
void init_fprime(void);

// Simulated F' main function - equivalent to your fprime_main
int fprime_main(int argc, char* argv[]) {
    printf("Fprime-style main starting...\n");
    
    // Initialize fprime components (simulated)
    init_fprime();
    
    printf("Entering F' main infinite loop\n");
    
    // The critical infinite loop
    unsigned int counter = 0;
    while (1) {
        if (counter % 10 == 0) {
            printf("F' style main heartbeat: %u\n", counter/10);
        }
        counter++;
        
        // Sleep for a bit
        rtems_task_wake_after(100); // 1 second at 100 ticks/sec
    }
    
    printf("This should never be reached!\n");
    return 0;
}

// Standard main that calls fprime_main - similar to your custom_init.c
int main(int argc, char* argv[]) {
    printf("Main function starting\n");
    printf("Calling fprime_main\n");
    
    return fprime_main(argc, argv);
}

// Simulated F' initialization function
void init_fprime(void) {
    printf("Initializing driver manager\n");
    
    // Initialize driver manager
    if (drvmgr_init() != 0) {
        printf("Driver manager initialization failed\n");
        exit(1);
    }
    
    printf("Driver manager initialized\n");
    printf("F' initialization complete\n");
}