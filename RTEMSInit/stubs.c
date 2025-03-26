#include <stdio.h>
#include <rtems.h>

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

// RTEMS-specific stubs
void* rtems_malloc(size_t size) {
    return malloc(size);
}

void* rtems_calloc(size_t nmemb, size_t size) {
    return calloc(nmemb, size);
}

void printk(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

// C++ runtime
void* __dso_handle = (void*)&__dso_handle;


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