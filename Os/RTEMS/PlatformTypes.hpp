#ifndef FPRIME_CONFIG_PLATFORMTYPES_H
#define FPRIME_CONFIG_PLATFORMTYPES_H

#include <stdint.h>
#include <sys/types.h>
#include <rtems.h>

/**
 * RTEMS Platform types for F′
 */

typedef int32_t         PlatformIntType;
#define PRI_PlatformIntType "d"

typedef uint32_t        PlatformUIntType;
#define PRI_PlatformUIntType "u"

typedef int32_t         PlatformIndexType;
#define PRI_PlatformIndexType "d"

typedef uint32_t        PlatformSizeType;
#define PRI_PlatformSizeType "u"

typedef uint32_t        PlatformPointerCastType;
#define PRI_PlatformPointerCastType "u"

typedef int32_t         PlatformAssertArgType;
#define PRI_PlatformAssertArgType "d"

// Define what types and checks are supported by this platform
#define FW_HAS_64_BIT 1                   //!< Architecture supports 64 bit integers
#define FW_HAS_32_BIT 1                   //!< Architecture supports 32 bit integers
#define FW_HAS_16_BIT 1                   //!< Architecture supports 16 bit integers
#define FW_HAS_F64 1                      //!< Architecture supports 64 bit floating point numbers
#define SKIP_FLOAT_IEEE_754_COMPLIANCE 0  //!<  Check IEEE 754 compliance of floating point arithmetic

#endif // FPRIME_CONFIG_PLATFORMTYPES_H