#include <stdio.h>
#include <rtems.h>
#include <stdarg.h>
#include <malloc.h>
#include <time.h>
#include <rtems/score/wkspace.h>
#include <rtems/score/heap.h>
#include <rtems/libcsupport.h>  // For region/heap support

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


// mallinfo implementation using RTEMS heap information
struct mallinfo mallinfo(void) {
    struct mallinfo info = {0};
    
    // Get heap information from RTEMS
    // The RTEMS heap is managed by the region manager
    extern Heap_Control *RTEMS_Malloc_Heap;
    
    if (RTEMS_Malloc_Heap) {
        Heap_Information_block heap_info;
        _Heap_Get_information(RTEMS_Malloc_Heap, &heap_info);
        
        // Fill mallinfo with data from RTEMS heap
        info.arena = heap_info.Stats.size;              // Total heap size
        info.ordblks = heap_info.Stats.free_blocks;     // Number of free blocks  
        info.fordblks = heap_info.Stats.free_size;      // Total free space
        info.uordblks = heap_info.Stats.size - heap_info.Stats.free_size;  // Used space
        info.usmblks = heap_info.Free.largest;          // Largest free block
        info.fsmblks = heap_info.Stats.min_free_size;   // Smallest free block
        info.hblks = 0;                                 // Not used
        info.hblkhd = 0;                                // Not used
        info.keepcost = 0;                              // Not applicable
    }
    
    return info;
}


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