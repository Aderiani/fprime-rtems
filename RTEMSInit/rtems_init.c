/*
 * Minimal RTEMS initialization for F Prime GR740 application
 */

/* Required RTEMS headers */
#include <rtems.h>
#include <bsp.h>

/* Standard C headers */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Network configuration structure */
typedef struct {
    int use_dhcp;
    const char* static_ip;
    const char* netmask;
    const char* gateway;
} NetworkConfig;

/* Function prototypes */
extern int fprime_main(int argc, char* argv[]);
NetworkConfig* create_network_config(int use_dhcp, const char* static_ip, const char* netmask, const char* gateway);
void cleanup_network_config(NetworkConfig* config);
int initialize_network(NetworkConfig* config);

/* Helper to duplicate strings */
static char* my_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

/* Network configuration API functions */
NetworkConfig* create_network_config(
    int use_dhcp, 
    const char* static_ip, 
    const char* netmask, 
    const char* gateway
) {
    NetworkConfig* config = malloc(sizeof(NetworkConfig));
    if (config) {
        config->use_dhcp = use_dhcp;
        config->static_ip = static_ip ? my_strdup(static_ip) : NULL;
        config->netmask = netmask ? my_strdup(netmask) : NULL;
        config->gateway = gateway ? my_strdup(gateway) : NULL;
    }
    return config;
}

void cleanup_network_config(NetworkConfig* config) {
    if (config) {
        free((void*)config->static_ip);
        free((void*)config->netmask);
        free((void*)config->gateway);
        free(config);
    }
}

/* Initialize network for GR740 board */
int initialize_network(NetworkConfig* config) {
    printf("Network initialization is not yet implemented\n");
    /* For now, we're just returning success without configuring the network */
    return 0;
}

/* RTEMS Task to run the F Prime application */
rtems_task Init(rtems_task_argument ignored) {
    printf("RTEMS initialized for F Prime on GR740\n");
    
    /* Call the F Prime main function */
    char *argv[] = {"fprime-gr740", NULL};
    int result = fprime_main(1, argv);
    
    printf("F Prime application exited with code: %d\n", result);
    rtems_task_suspend(RTEMS_SELF);
}

/****************** RTEMS Minimal Configuration **********************/

#define CONFIGURE_MINIMUM_TASKS_WITH_USER_PROVIDED_STORAGE

rtems_initialization_tasks_table Initialization_tasks[] = {
  { rtems_build_name('I', 'N', 'I', 'T'),
    RTEMS_MINIMUM_STACK_SIZE * 8,
    1,
    RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT,
    Init,
    RTEMS_DEFAULT_MODES,
    0
  }
};

#define CONFIGURE_INIT
#define CONFIGURE_INIT_TASK_TABLE Initialization_tasks
#define CONFIGURE_INIT_TASK_TABLE_SIZE \
  (sizeof(Initialization_tasks) / sizeof(rtems_initialization_tasks_table))

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS 32
#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 32
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_PERIODS 4
#define CONFIGURE_MAXIMUM_REGIONS 2
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 2
#define CONFIGURE_MAXIMUM_TIMERS 8

#define CONFIGURE_MICROSECONDS_PER_TICK 1000
#define CONFIGURE_TICKS_PER_TIMESLICE 50
#define CONFIGURE_APPLICATION_DOES_NOT_NEED_STRUCT_TIMESPEC

/* Include the minimal RTEMS configuration */
#include <rtems/confdefs.h>