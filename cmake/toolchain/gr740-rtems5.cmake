####
# gr740-rtems5.cmake:
#
# Toolchain file setup for building F prime for the GR740 hardware platform.
#
####
include_guard()
set(CMAKE_SYSTEM_NAME         RTEMS)
set(CMAKE_SYSTEM_VERSION      5)
set(CMAKE_SYSTEM_PROCESSOR    sparc)
set(CMAKE_TRY_COMPILE_TARGET_TYPE
    "STATIC_LIBRARY"
    CACHE STRING "Try Static Lib Type" FORCE)

# Adapt the path to your specific RTEMS installation
set(TOOLCHAIN_PATH "/opt/rtems/rcc-1.3.2-gcc" CACHE PATH "Toolchain path")
set(RTEMS_BSP "gr740_smp" CACHE STRING "RTEMS BSP")

# Check toolchain directory exists
IF(NOT EXISTS "${TOOLCHAIN_PATH}")
    message(FATAL_ERROR "Toolchain not found at ${TOOLCHAIN_PATH}.")
endif()
message(STATUS "Using toolchain at: ${TOOLCHAIN_PATH}")
message(STATUS "Using RTEMS BSP: ${RTEMS_BSP}")

set(CROSS_PREFIX "sparc-gaisler-rtems5")
set(CROSS_SUFFIX "")

# Specify the cross toolchain executables
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-gcc${CROSS_SUFFIX}"     CACHE PATH "assembler"  FORCE)
set(CMAKE_C_COMPILER   "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-gcc${CROSS_SUFFIX}"     CACHE PATH "gcc"        FORCE)
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-g++${CROSS_SUFFIX}"     CACHE PATH "g++"        FORCE)
set(CMAKE_LINKER       "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-ld${CROSS_SUFFIX}"      CACHE PATH "linker"     FORCE)
set(CMAKE_AR           "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-ar${CROSS_SUFFIX}"      CACHE PATH "archive"    FORCE)
set(CMAKE_NM           "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-nm${CROSS_SUFFIX}"      CACHE PATH "nm"         FORCE)
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-objcopy${CROSS_SUFFIX}" CACHE PATH "objcopy"    FORCE)
set(CMAKE_OBJDUMP      "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-objdump${CROSS_SUFFIX}" CACHE PATH "objdump"    FORCE)
set(CMAKE_STRIP        "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-strip${CROSS_SUFFIX}"   CACHE PATH "strip"      FORCE)
set(CMAKE_SIZE         "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-size${CROSS_SUFFIX}"    CACHE PATH "size"       FORCE)
set(CMAKE_RANLIB       "${TOOLCHAIN_PATH}/bin/${CROSS_PREFIX}-ranlib${CROSS_SUFFIX}"  CACHE PATH "ranlib"     FORCE)

# GR740-specific flags - adjusted for your CPU
set(ISA_FLAG "-mcpu=leon3")
set(COMMON_FLAGS "-g ${ISA_FLAG}")

# Define compile flags
set(DEF_FLAGS "")
set(C_CXX_FLAGS "-O2 -ffunction-sections -fdata-sections -Wall")

set(C_FLAGS "-Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes -Wnested-externs")
set(CXX_FLAGS "-fno-exceptions -fno-rtti")
set(ASM_FLAGS "-x assembler-with-cpp")
set(LD_FLAGS "-Wl,--gc-sections")

# Set the flags for each language
set(CMAKE_C_FLAGS           "${COMMON_FLAGS} ${C_CXX_FLAGS} ${C_FLAGS} ${DEF_FLAGS}" CACHE STRING "CFLAGS" FORCE)
set(CMAKE_CXX_FLAGS         "${COMMON_FLAGS} ${C_CXX_FLAGS} ${CXX_FLAGS} ${DEF_FLAGS}" CACHE STRING "CXXFLAGS" FORCE)
set(CMAKE_ASM_FLAGS         "${COMMON_FLAGS} ${ASM_FLAGS} ${DEF_FLAGS}" CACHE STRING "ASMFLAGS" FORCE)
set(CMAKE_EXE_LINKER_FLAGS  "${COMMON_FLAGS} ${LD_FLAGS}" CACHE STRING "LDFLAGS" FORCE)

# RTEMS BSP specific paths
set(RTEMS_BSP_PATH "${TOOLCHAIN_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}/${RTEMS_BSP}")

# Add BSP specific include paths
include_directories(
    ${RTEMS_BSP_PATH}/lib/include
    ${TOOLCHAIN_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}/include
)

# Set search paths
set(CMAKE_FIND_ROOT_PATH 
    ${RTEMS_BSP_PATH}
    ${TOOLCHAIN_PATH}/sparc-gaisler-rtems${CMAKE_SYSTEM_VERSION}
    ${TOOLCHAIN_PATH}
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