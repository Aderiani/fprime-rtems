#include <bsp.h>
#include <rtems.h>

void _init(void) {
    bsp_start();    // Initialize BSP
    boot_card(0);   // Start RTEMS executive
}