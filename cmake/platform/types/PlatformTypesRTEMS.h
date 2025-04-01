#ifndef PLATFORM_TYPES_RTEMS_H_
#define PLATFORM_TYPES_RTEMS_H_

#include <inttypes.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FW_HAS_64_BIT 1
#define FW_HAS_32_BIT 1
#define FW_HAS_16_BIT 1
#define FW_HAS_F64 1
#define SKIP_FLOAT_IEEE_754_COMPLIANCE 0

typedef int PlatformIntType;
#define PRI_PlatformIntType "d"

typedef unsigned int PlatformUIntType;
#define PRI_PlatformUIntType "u"

typedef PlatformIntType PlatformIndexType;
#define PRI_PlatformIndexType PRI_PlatformIntType

typedef int64_t PlatformSignedSizeType;
#define PRI_PlatformSignedSizeType PRId64

typedef uint64_t PlatformSizeType;
#define PRI_PlatformSizeType PRIu64

typedef PlatformIntType PlatformAssertArgType;
#define PRI_PlatformAssertArgType PRI_PlatformIntType

typedef PlatformIntType PlatformTaskPriorityType;
#define PRI_PlatformTaskPriorityType PRI_PlatformIntType

typedef PlatformIntType PlatformQueuePriorityType;
#define PRI_PlatformQueuePriorityType PRI_PlatformIntType

typedef uint32_t PlatformPointerCastType; // 32-bit pointers on SPARC
#define PRI_PlatformPointerCastType PRIx32

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_TYPES_RTEMS_H_