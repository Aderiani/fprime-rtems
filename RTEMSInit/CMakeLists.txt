# RTEMSInit/CMakeLists.txt
set(RTEMS_PREFIX "/opt/rtems/rcc-1.3.2-gcc/sparc-gaisler-rtems5/gr740_smp")

add_library(RTEMSInit
    rtems_init.c
    network_init.c
    bsp_init.c
    stubs.c
)

target_include_directories(RTEMSInit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${RTEMS_PREFIX}/lib/include
)

target_link_directories(RTEMSInit PUBLIC
    ${RTEMS_PREFIX}/lib
)

target_link_libraries(RTEMSInit PUBLIC
    ${RTEMS_PREFIX}/lib/start.o  # Startup code
    rtemsbsp                     # BSP library
    rtemscpu                     # CPU library (contains sparc_syscall_exit)
    c                            # Standard C library
)