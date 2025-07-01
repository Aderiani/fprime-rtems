####
# gr740-rtems5.cmake:
# Toolchain file setup for building F prime for the GR740 hardware platform.
# This toolchain links with RTEMS 5 for the GR740 SMP BSP.
####
include_guard()
set(CMAKE_SYSTEM_NAME         "RTEMS")
set(CMAKE_SYSTEM_VERSION      5)
set(CMAKE_SYSTEM_PROCESSOR    sparc)
set(FPRIME_PLATFORM "RTEMS" CACHE STRING "Platform for the build" FORCE)

# Define RTEMS as the target OS type
add_definitions(-DCMAKE_SYSTEM_NAME_RTEMS=1)
add_definitions(-DTGT_OS_TYPE_RTEMS=1)
add_definitions(-D__rtems__=1)
add_definitions(-DRTEMS_DRVMGR_STARTUP)


# Define these for queue debugging and verbose output
add_definitions(-DDEBUG_QUEUE=1)
add_definitions(-DRTEMS_VERBOSE=1)

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

set(RTEMS_BSP_PATH "${RTEMS_PATH}/${CROSS_PREFIX}/${RTEMS_BSP}")

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

set(CMAKE_INCLUDE_PATH ${RTEMS_BSP_PATH}/lib/include)
set(CMAKE_LIBRARY_PATH ${RTEMS_BSP_PATH}/lib)

# # GR740-specific flags
set(ISA_FLAG "-mcpu=leon3")
set(COMMON_FLAGS "-g ${ISA_FLAG}")

# Define compile flags
set(DEF_FLAGS "-DTGT_OS_TYPE_RTEMS -D__rtems__")
set(C_CXX_FLAGS "-O2 -ffunction-sections -fdata-sections -Wall")

set(C_FLAGS "-Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes -Wnested-externs")
set(CXX_FLAGS "-fno-exceptions -fno-rtti")

# Error-suppressing flags for RTEMS compatibility
set(COMPAT_FLAGS "-Wno-pedantic -Wno-error=ignored-qualifiers -Wno-error=old-style-cast -Wno-error=sign-compare")
add_compile_options(
    -Wno-shadow 
    -Wno-error=shadow 
    -fno-common
    -Wno-implicit-function-declaration
    -Wno-error=implicit-function-declaration
)

#TODO: This is added just to avoid error in size casting for posix rtems. The error shall be addressed and this should be removed.
# In cmake/platform/RTEMS.cmake, add these to your compiler flags:
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wno-error=pedantic -Wno-error=shadow -Wno-error=unused-variable -Wno-error=attributes -Wno-error=old-style-cast -Wno-error=conversion"
)

# And similarly for C:
set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -Wno-error=pedantic -Wno-error=shadow -Wno-error=unused-variable -Wno-error=attributes -Wno-error=conversion"
)

# You can also add this directly to compile options:
add_compile_options(
    -Wno-error=conversion
)




# Set the flags for each language
set(CMAKE_C_FLAGS           "${COMMON_FLAGS} ${C_CXX_FLAGS} ${C_FLAGS} ${DEF_FLAGS} ${COMPAT_FLAGS}" CACHE STRING "CFLAGS" FORCE)
set(CMAKE_CXX_FLAGS         "${COMMON_FLAGS} ${C_CXX_FLAGS} ${CXX_FLAGS} ${DEF_FLAGS} ${COMPAT_FLAGS}" CACHE STRING "CXXFLAGS" FORCE)
set(CMAKE_ASM_FLAGS         "${COMMON_FLAGS} ${ASM_FLAGS} ${DEF_FLAGS}" CACHE STRING "ASMFLAGS" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")


# Add RTEMS pthread support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qbsp=gr740_smp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qbsp=gr740_smp")

# Linker flags for RTEMS
set(RTEMS_LDFLAGS "-Wl,--gc-sections -Wl,--wrap=printf -Wl,--wrap=puts -Wl,--wrap=putchar")

set(CMAKE_EXE_LINKER_FLAGS "${COMMON_FLAGS} ${RTEMS_LDFLAGS}" CACHE STRING "LDFLAGS" FORCE)
# Add executable suffix
set(CMAKE_EXECUTABLE_SUFFIX ".exe" CACHE STRING "Executable suffix" FORCE)

# RTEMS BSP specific include paths
include_directories(SYSTEM
    ${RTEMS_PATH}/${CROSS_PREFIX}/include
    ${RTEMS_BSP_PATH}/lib/include
    ${RTEMS_BSP_PATH}/lib/include/bsp
    ${RTEMS_BSP_PATH}/lib/include/networking
    ${RTEMS_BSP_PATH}/lib/include/grlib
)

# Add RTEMS library paths
link_directories(
    ${RTEMS_BSP_PATH}/lib
    ${RTEMS_PATH}/${CROSS_PREFIX}/lib
    ${RTEMS_PATH}/lib/gcc/${CROSS_PREFIX}/10.5.0/leon3
)

# Set search paths
set(CMAKE_FIND_ROOT_PATH 
    ${RTEMS_BSP_PATH}
    ${RTEMS_PATH}/${CROSS_PREFIX}
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

# Add RTEMS libraries needed for linking
set(RTEMS_LIBS
    rtemscpu
    rtemsbsp
    c
    m
    gcc
)

# Add the libraries to the link line
foreach(lib ${RTEMS_LIBS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -l${lib}")
endforeach()