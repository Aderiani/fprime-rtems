####
# gr740-rtems5.cmake:
# Toolchain file setup for building F prime for the GR740 hardware platform.
# This toolchain links with RTEMS 5 for the GR740 SMP BSP.
####
include_guard()
set(CMAKE_SYSTEM_NAME         "RTEMS")
set(CMAKE_SYSTEM_VERSION      5)
set(CMAKE_SYSTEM_PROCESSOR    sparc)
set(FPRIME_PLATFORM           RTEMS)

# Define RTEMS as the target OS type
add_definitions(-DCMAKE_SYSTEM_NAME_RTEMS=1)
add_definitions(-DTGT_OS_TYPE_RTEMS=1)
add_definitions(-DRTEMS_VERSION_5=1)

set(CMAKE_TRY_COMPILE_TARGET_TYPE
    "STATIC_LIBRARY"
    CACHE STRING "Try Static Lib Type" FORCE)

# Path to your RTEMS installation
set(RTEMS_PATH "/opt/rtems/rcc-1.3.2-gcc" CACHE PATH "RTEMS installation path")
set(RTEMS_BSP "gr740_smp" CACHE STRING "RTEMS BSP")

# Check toolchain directory exists
if(NOT EXISTS "${RTEMS_PATH}")
    message(FATAL_ERROR "RTEMS toolchain not found at ${RTEMS_PATH}.")
endif()
message(STATUS "Using RTEMS toolchain at: ${RTEMS_PATH}")
message(STATUS "Using RTEMS BSP: ${RTEMS_BSP}")

set(CROSS_PREFIX "sparc-gaisler-rtems5")
set(CROSS_SUFFIX "")

# Specify the cross toolchain executables
set(CMAKE_ASM_COMPILER "${RTEMS_PATH}/bin/${CROSS_PREFIX}-gcc${CROSS_SUFFIX}"     CACHE PATH "assembler"  FORCE)
set(CMAKE_C_COMPILER   "${RTEMS_PATH}/bin/${CROSS_PREFIX}-gcc${CROSS_SUFFIX}"     CACHE PATH "gcc"        FORCE)
set(CMAKE_CXX_COMPILER "${RTEMS_PATH}/bin/${CROSS_PREFIX}-g++${CROSS_SUFFIX}"     CACHE PATH "g++"        FORCE)
set(CMAKE_LINKER       "${RTEMS_PATH}/bin/${CROSS_PREFIX}-ld${CROSS_SUFFIX}"      CACHE PATH "linker"     FORCE)
set(CMAKE_AR           "${RTEMS_PATH}/bin/${CROSS_PREFIX}-ar${CROSS_SUFFIX}"      CACHE PATH "archive"    FORCE)
set(CMAKE_NM           "${RTEMS_PATH}/bin/${CROSS_PREFIX}-nm${CROSS_SUFFIX}"      CACHE PATH "nm"         FORCE)
set(CMAKE_OBJCOPY      "${RTEMS_PATH}/bin/${CROSS_PREFIX}-objcopy${CROSS_SUFFIX}" CACHE PATH "objcopy"    FORCE)
set(CMAKE_OBJDUMP      "${RTEMS_PATH}/bin/${CROSS_PREFIX}-objdump${CROSS_SUFFIX}" CACHE PATH "objdump"    FORCE)
set(CMAKE_STRIP        "${RTEMS_PATH}/bin/${CROSS_PREFIX}-strip${CROSS_SUFFIX}"   CACHE PATH "strip"      FORCE)
set(CMAKE_SIZE         "${RTEMS_PATH}/bin/${CROSS_PREFIX}-size${CROSS_SUFFIX}"    CACHE PATH "size"       FORCE)
set(CMAKE_RANLIB       "${RTEMS_PATH}/bin/${CROSS_PREFIX}-ranlib${CROSS_SUFFIX}"  CACHE PATH "ranlib"     FORCE)

# GR740-specific flags
set(ISA_FLAG "-mcpu=leon3")
set(COMMON_FLAGS "-g ${ISA_FLAG}")

# Define compile flags
set(DEF_FLAGS "-DTGT_OS_TYPE_RTEMS")
set(C_CXX_FLAGS "-O2 -ffunction-sections -fdata-sections -Wall")

set(C_FLAGS "-Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes -Wnested-externs")
set(CXX_FLAGS "-fno-exceptions -fno-rtti")

# Error-suppressing flags for RTEMS compatibility
set(COMPAT_FLAGS "-Wno-pedantic -Wno-error=ignored-qualifiers -Wno-error=old-style-cast -Wno-error=sign-compare")
add_compile_options(-Wno-shadow -Wno-error=shadow -fno-common)

# Set the flags for each language
set(CMAKE_C_FLAGS           "${COMMON_FLAGS} ${C_CXX_FLAGS} ${C_FLAGS} ${DEF_FLAGS} ${COMPAT_FLAGS}" CACHE STRING "CFLAGS" FORCE)
set(CMAKE_CXX_FLAGS         "${COMMON_FLAGS} ${C_CXX_FLAGS} ${CXX_FLAGS} ${DEF_FLAGS} ${COMPAT_FLAGS}" CACHE STRING "CXXFLAGS" FORCE)
set(CMAKE_ASM_FLAGS         "${COMMON_FLAGS} ${ASM_FLAGS} ${DEF_FLAGS}" CACHE STRING "ASMFLAGS" FORCE)

# Add RTEMS pthread support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit")

# Linker flags - these are critical for RTEMS
set(RTEMS_LDFLAGS "-Wl,--gc-sections -Wl,-znorelro -Wl,--wrap=printf -Wl,--wrap=putchar")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMMON_FLAGS} ${RTEMS_LDFLAGS}" CACHE STRING "LDFLAGS" FORCE)
set(CMAKE_EXECUTABLE_SUFFIX ".exe" CACHE STRING "Executable suffix" FORCE)

# Link RTEMS libraries
set(RTEMS_LIBS 
    rtemscpu
    rtemsbsp 
    posix
)


# Required for C++ programs
set(CMAKE_CXX_STANDARD_LIBRARIES "-lc -lm -lgcc -lrtemscpu -lrtemsbsp")

# RTEMS BSP specific paths
set(RTEMS_BSP_PATH "${RTEMS_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}/${RTEMS_BSP}")

# Add BSP specific include paths
include_directories(SYSTEM
    ${RTEMS_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}/include
    ${RTEMS_BSP_PATH}/lib/include
    ${RTEMS_BSP_PATH}/lib/include/bsp
    ${RTEMS_BSP_PATH}/lib/include/networking
    ${RTEMS_BSP_PATH}/lib/include/grlib
)

# Add RTEMS library paths
link_directories(
    ${RTEMS_BSP_PATH}/lib
    ${RTEMS_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}/lib
    ${RTEMS_PATH}/lib/gcc/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}/10.5.0/leon3
)


# Set search paths
set(CMAKE_FIND_ROOT_PATH 
    ${RTEMS_BSP_PATH}
    ${RTEMS_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}
    ${RTEMS_PATH}
)

# Specify paths for finding programs/libraries/includes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set language standards
foreach(lang IN ITEMS C CXX)
  # Specify the C and C++ standard required for ALL targets
  set(CMAKE_${lang}_STANDARD
      11
      CACHE
        INTERNAL
        "The ${lang} standard whose features are requested to build this target."
  )
  set(CMAKE_${lang}_STANDARD_REQUIRED ON)
  set(CMAKE_${lang}_EXTENSIONS OFF CACHE INTERNAL "${lang} compiler extensions OFF")
endforeach()

# Make sure we use static linking
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)