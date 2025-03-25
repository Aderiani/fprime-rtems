void* __dso_handle = nullptr;

// Define RTEMS_NO_TIMEOUT if needed
#ifndef RTEMS_NO_TIMEOUT
#define RTEMS_NO_TIMEOUT 0
#endif

// Implement minimal pthread_once functionality if needed
extern "C" {
    // Define pthread_once_t if needed by your code
    typedef int pthread_once_t;
    
    int pthread_once(pthread_once_t* once_control, void (*init_routine)(void)) {
        if (*once_control == 0) {
            *once_control = 1;
            if (init_routine) {
                init_routine();
            }
        }
        return 0;
    }
}