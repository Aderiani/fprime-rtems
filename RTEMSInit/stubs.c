#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/bspIo.h>

// I/O Stubs
int __wrap_puts(const char* s) { return puts(s); }
int __wrap_putchar(int c) { return putchar(c); }
int __