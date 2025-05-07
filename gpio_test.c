/*
 * GR740 GPIO LED Blinking Test - Direct register access approach
 */

 #include <rtems.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <bsp.h>
 #include <sys/mman.h>
 #include <fcntl.h>
 #include <unistd.h>
 
 /* Configuration information */
 #define CONFIGURE_INIT
 
 #include <bsp.h> /* for device driver prototypes */
 
 rtems_task Init(rtems_task_argument argument);
 
 /* Configuration information */
 #define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
 #define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
 
 #define CONFIGURE_MAXIMUM_TASKS             4
 #define CONFIGURE_RTEMS_INIT_TASKS_TABLE
 #define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
 #define CONFIGURE_MAXIMUM_SEMAPHORES        4
 #define CONFIGURE_INIT_TASK_ATTRIBUTES      RTEMS_FLOATING_POINT
 
 #include <rtems/confdefs.h>
 
 /* From your debug output, we see these GPIO controller addresses */
 #define GPIO0_BASE_ADDR 0xffa08000    /* First controller */
 #define GPIO1_BASE_ADDR 0xff902000    /* Second controller */
 
 /* Offset for output register */
 #define GPIO_DATA_OUT_OFFSET 0x04
 #define GPIO_DIRECTION_OFFSET 0x08
 
 /* Define the number of seconds for the pattern */
 #define ON_SECONDS   3
 #define OFF_SECONDS  3
 
 /* LED control bits - according to your documentation */
 #define LED7_BIT  0x20  /* Bit 5 */
 #define LED8_BIT  0x40  /* Bit 6 */
 #define LED_MASK  0x60  /* Both LEDs */
 
 rtems_task Init(rtems_task_argument ignored)
 {
     rtems_interval ticks_per_second;
     int fd;
     void *gpio_map;
     volatile unsigned int *gpio0_data_reg;
     volatile unsigned int *gpio1_data_reg;
     volatile unsigned int *gpio0_dir_reg;
     volatile unsigned int *gpio1_dir_reg;
     unsigned int val;
     
     printf("Starting GR740 GPIO LED Blink Test - Direct Register Approach\n");
     
     /* Open /dev/mem for direct memory access */
     fd = open("/dev/mem", O_RDWR);
     if (fd < 0) {
         printf("Failed to open /dev/mem. Trying different approach...\n");
         
         /* Direct volatile pointer approach as fallback */
         gpio0_data_reg = (volatile unsigned int *)(GPIO0_BASE_ADDR + GPIO_DATA_OUT_OFFSET);
         gpio0_dir_reg = (volatile unsigned int *)(GPIO0_BASE_ADDR + GPIO_DIRECTION_OFFSET);
         gpio1_data_reg = (volatile unsigned int *)(GPIO1_BASE_ADDR + GPIO_DATA_OUT_OFFSET);
         gpio1_dir_reg = (volatile unsigned int *)(GPIO1_BASE_ADDR + GPIO_DIRECTION_OFFSET);
     } else {
         /* Map both GPIO controllers */
         gpio_map = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO0_BASE_ADDR);
         if (gpio_map == MAP_FAILED) {
             printf("Failed to map GPIO0 controller\n");
             close(fd);
             exit(1);
         }
         gpio0_data_reg = (volatile unsigned int *)((char *)gpio_map + GPIO_DATA_OUT_OFFSET);
         gpio0_dir_reg = (volatile unsigned int *)((char *)gpio_map + GPIO_DIRECTION_OFFSET);
         
         gpio_map = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO1_BASE_ADDR);
         if (gpio_map == MAP_FAILED) {
             printf("Failed to map GPIO1 controller\n");
             close(fd);
             exit(1);
         }
         gpio1_data_reg = (volatile unsigned int *)((char *)gpio_map + GPIO_DATA_OUT_OFFSET);
         gpio1_dir_reg = (volatile unsigned int *)((char *)gpio_map + GPIO_DIRECTION_OFFSET);
     }
     
     /* Configure GPIO direction - set LEDs as outputs */
     /* Try both controllers */
     
     /* Read current direction values */
     printf("GPIO0 direction register initial value: 0x%08x\n", *gpio0_dir_reg);
     printf("GPIO1 direction register initial value: 0x%08x\n", *gpio1_dir_reg);
     
     /* Set LED bits as outputs - try both controllers */
     *gpio0_dir_reg |= LED_MASK;  /* Set bits 5 and 6 as outputs in controller 0 */
     *gpio1_dir_reg |= LED_MASK;  /* Set bits 5 and 6 as outputs in controller 1 */
     
     printf("GPIO0 direction register after setting bits: 0x%08x\n", *gpio0_dir_reg);
     printf("GPIO1 direction register after setting bits: 0x%08x\n", *gpio1_dir_reg);
     
     /* Read initial output values */
     printf("GPIO0 output register initial value: 0x%08x\n", *gpio0_data_reg);
     printf("GPIO1 output register initial value: 0x%08x\n", *gpio1_data_reg);
     
     /* Get ticks per second for delay calculations */
     ticks_per_second = rtems_clock_get_ticks_per_second();
     
     printf("LED Blink Test: Starting continuous %d-second on, %d-second off pattern\n", 
            ON_SECONDS, OFF_SECONDS);
     
     /* Continuous blinking loop - try both controllers */
     while (1) {
         /* Turn LEDs ON */
         printf("Turning LEDs 7 & 8 ON\n");
         
         /* Set bits 5 and 6 high in both controllers */
         *gpio0_data_reg |= LED_MASK;  /* Set bits in controller 0 */
         *gpio1_data_reg |= LED_MASK;  /* Set bits in controller 1 */
         
         printf("GPIO0 output register after setting LEDs: 0x%08x\n", *gpio0_data_reg);
         printf("GPIO1 output register after setting LEDs: 0x%08x\n", *gpio1_data_reg);
         
         /* Delay for ON_SECONDS */
         rtems_task_wake_after(ticks_per_second * ON_SECONDS);
         
         /* Turn LEDs OFF */
         printf("Turning LEDs 7 & 8 OFF\n");
         
         /* Clear bits 5 and 6 in both controllers */
         *gpio0_data_reg &= ~LED_MASK;  /* Clear bits in controller 0 */
         *gpio1_data_reg &= ~LED_MASK;  /* Clear bits in controller 1 */
         
         printf("GPIO0 output register after clearing LEDs: 0x%08x\n", *gpio0_data_reg);
         printf("GPIO1 output register after clearing LEDs: 0x%08x\n", *gpio1_data_reg);
         
         /* Delay for OFF_SECONDS */
         rtems_task_wake_after(ticks_per_second * OFF_SECONDS);
         
         /* Now try alternating LEDs */
         printf("Turning LED 7 ON, LED 8 OFF\n");
         
         /* Set bit 5 and clear bit 6 in both controllers */
         *gpio0_data_reg = (*gpio0_data_reg & ~LED_MASK) | LED7_BIT;
         *gpio1_data_reg = (*gpio1_data_reg & ~LED_MASK) | LED7_BIT;
         
         printf("GPIO0 output register with LED 7 ON: 0x%08x\n", *gpio0_data_reg);
         printf("GPIO1 output register with LED 7 ON: 0x%08x\n", *gpio1_data_reg);
         
         /* Delay for ON_SECONDS */
         rtems_task_wake_after(ticks_per_second * ON_SECONDS);
         
         printf("Turning LED 7 OFF, LED 8 ON\n");
         
         /* Clear bit 5 and set bit 6 in both controllers */
         *gpio0_data_reg = (*gpio0_data_reg & ~LED_MASK) | LED8_BIT;
         *gpio1_data_reg = (*gpio1_data_reg & ~LED_MASK) | LED8_BIT;
         
         printf("GPIO0 output register with LED 8 ON: 0x%08x\n", *gpio0_data_reg);
         printf("GPIO1 output register with LED 8 ON: 0x%08x\n", *gpio1_data_reg);
         
         /* Delay for ON_SECONDS */
         rtems_task_wake_after(ticks_per_second * ON_SECONDS);
     }
     
     /* Cleanup if ever reached */
     close(fd);
     
     exit(0);
 }