#include <stdio.h>
#include <rtems.h>
#include <stdarg.h>
#include <malloc.h>
#include <time.h>
#include <rtems/score/wkspace.h>

// Wrapped functions
int __wrap_puts(const char* s) {
    return puts(s);
}

int __wrap_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    return ret;
}

int __wrap_putchar(int c) {
    return putchar(c);
}

// POSIX stubs (temporary, until POSIX is enabled)
int pthread_once(void* once_control, void (*init_routine)(void)) {
    static int done = 0;
    if (!done) {
        init_routine();
        done = 1;
    }
    return 0;
}

int pthread_key_create(void* key, void (*destructor)(void*)) {
    return 0;  // Stub: no thread-specific data support
}

int pthread_key_delete(void* key) {
    return 0;  // Stub
}

void* pthread_getspecific(void* key) {
    return NULL;  // Stub
}

int pthread_setspecific(void* key, const void* value) {
    return 0;  // Stub
}

// mallinfo implementation using RTEMS workspace information
struct mallinfo mallinfo(void) {
    struct mallinfo info = {0};
    
    // Get workspace (heap) information from RTEMS
    Heap_Information_block heap_info;
    bool result = _Workspace_Get_information(&heap_info);
    
    if (result) {
        // Fill mallinfo with data from RTEMS workspace
        // The workspace is essentially the heap in RTEMS
        info.arena = heap_info.Stats.size;              // Total heap size
        info.ordblks = heap_info.Stats.free_blocks;     // Number of free blocks  
        info.fordblks = heap_info.Stats.free_size;      // Total free space
        // Calculate used space = total - free
        info.uordblks = heap_info.Stats.size - heap_info.Stats.free_size;
        info.hblks = 0;                                 // Not used in RTEMS
        info.hblkhd = 0;                                // Not used in RTEMS
        // Use available free size info
        info.usmblks = heap_info.Free.largest;          // Largest free block
        info.fsmblks = 4096;                            // Minimum allocation size (guess)
        info.keepcost = 0;                              // Not applicable
    } else {
        // If we can't get workspace info, return configured values
        // This matches your CONFIGURE_EXECUTIVE_RAM_SIZE
        info.arena = 200 * 1024 * 1024;    // 200MB total
        info.fordblks = 150 * 1024 * 1024; // 150MB free (estimate)
        info.uordblks = 50 * 1024 * 1024;  // 50MB used (estimate)
        info.ordblks = 100;                // Some free blocks
        info.usmblks = 10 * 1024 * 1024;  // 10MB largest free
        info.fsmblks = 4096;               // 4KB minimum
    }
    
    return info;
}

// printk is already defined in RTEMS, remove our implementation
// void printk(const char* fmt, ...) {
//     va_list args;
//     va_start(args, fmt);
//     vprintf(fmt, args);
//     va_end(args);
// }

// Override printf, puts, putchar with our implementations
int printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    return ret;
}

int puts(const char* s) {
    return fputs(s, stdout) + putchar('\n');
}

int putchar(int c) {
    return fputc(c, stdout);
}