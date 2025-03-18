// File: Drv/RTEMS/GR740/include/gr740_gpio.h
#ifndef GR740_GPIO_H
#define GR740_GPIO_H

// Include RTEMS headers only within the extern "C" block
#ifdef __cplusplus
extern "C" {
#endif

// GRGPIO Register Map
typedef struct {
    volatile unsigned int data;      /* I/O port data register */
    volatile unsigned int output;    /* I/O port output register */
    volatile unsigned int dir;       /* I/O port direction register */
    volatile unsigned int imask;     /* Interrupt mask register */
    volatile unsigned int ipol;      /* Interrupt polarity register */
    volatile unsigned int edge;      /* Interrupt edge register */
    volatile unsigned int bypass;    /* Bypass register */
} gr740_gpio_regs;

// GRGPIO Register bit definitions
#define GRGPIO_DATA           0x00    /* 0x00 Data register */
#define GRGPIO_OUTPUT         0x04    /* 0x04 Output register */
#define GRGPIO_DIRECTION      0x08    /* 0x08 Direction register */
#define GRGPIO_IMASK          0x0C    /* 0x0C Interrupt mask register */
#define GRGPIO_IPOL           0x10    /* 0x10 Interrupt polarity register */
#define GRGPIO_IEDGE          0x14    /* 0x14 Interrupt edge register */
#define GRGPIO_BYPASS         0x18    /* 0x18 Bypass register */

#ifdef __cplusplus
}
#endif

#endif // GR740_GPIO_H