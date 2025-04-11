
#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE  // For compatibility with older versions of RTEMS
#endif

#include "rtems_config.h"
#include <drvmgr/drvmgr_confdefs.h>
#include <rtems.h>
#include <rtems/confdefs.h>
#include <stddef.h>
#include <grlib/network_interface_add.h>  // This file defines the ethernet_config struct



struct ethernet_config interface_configs[] = {
    { "192.168.0.67", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x79}},
    { NULL, NULL, {0,0,0,0,0,0}}
};