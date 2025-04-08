#ifndef _RTEMS_NETWORKCONFIG_H_
#define _RTEMS_NETWORKCONFIG_H_

#include <grlib/ambapp_bus_grlib.h>
#include <grlib/ambapp_bus.h>
#include <grlib/ambapp_ids.h>
#include <grlib/network_interface_add.h>

// Declare the interface_configs array that will be used by network_interface_add
extern struct ethernet_config interface_configs[];

// System initialization function declaration
void system_init2(void);

#endif /* _RTEMS_NETWORKCONFIG_H_ */