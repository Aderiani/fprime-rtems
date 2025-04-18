/**
 * RTEMS Application Configuration - contains ALL RTEMS configuration
 * for the entire application.
 */
#include <rtems.h>
#include <stdio.h>

extern int fprime_main(int argc, char* argv[]);

/* Standard C main entry point expected by RTEMS */
int main(int argc, char* argv[]) {
    return fprime_main(argc, argv);
}
