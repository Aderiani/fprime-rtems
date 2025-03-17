// File: Drv/RTEMS/GR740/include/gr740_gpio.h
#ifndef GR740_GPIO_H
#define GR740_GPIO_H

#include <FpConfig.hpp>

#ifdef __cplusplus
extern "C" {
#endif

// GRGPIO Register Map
typedef struct {
    volatile U32 data;      /* I/O port data register */
    volatile U32 output;    /* I/O port output register */
    volatile U32 dir;       /* I/O port direction register */
    volatile U32 imask;     /* Interrupt mask register */
    volatile U32 ipol;      /* Interrupt polarity register */
    volatile U32 edge;      /* Interrupt edge register */
    volatile U32 bypass;    /* Bypass register */
} gr740_gpio_regs;

// Structure to access GPIO registers
typedef struct {
    volatile gr740_gpio_regs* regs;
} gr740_gpio_device;

// GRGPIO Register bit definitions
#define GRGPIO_DATA           0x00    /* 0x00 Data register */
#define GRGPIO_OUTPUT         0x04    /* 0x04 Output register */
#define GRGPIO_DIRECTION      0x08    /* 0x08 Direction register */
#define GRGPIO_IMASK          0x0C    /* 0x0C Interrupt mask register */
#define GRGPIO_IPOL           0x10    /* 0x10 Interrupt polarity register */
#define GRGPIO_IEDGE          0x14    /* 0x14 Interrupt edge register */
#define GRGPIO_BYPASS         0x18    /* 0x18 Bypass register */
#define GRGPIO_STATUS         0x1C    /* 0x1C interrupt and capability status register */
#define GRGPIO_IRQMAP         0x20    /* 0x20 Interrupt map register */

#ifdef __cplusplus
}
#endif

#endif // GR740_GPIO_H