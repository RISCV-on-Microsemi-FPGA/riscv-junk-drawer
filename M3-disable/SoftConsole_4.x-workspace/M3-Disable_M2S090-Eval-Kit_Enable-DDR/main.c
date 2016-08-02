/*******************************************************************************
 * (c) Copyright 2016 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 microcontroller subsystem (MSS) Cortex-M3 disable example
 * program.
 * This program is intended to configure the MSS DDR memory controller and
 * SERDES block(s) before making the Cortex-M3 non operative leaving full access
 * to MSS peripherals from an FPGA fabric master.
 *
 * Please refer to the file README.txt for further details about this example.
 */
#include "drivers/mss_gpio/mss_gpio.h"
#include "CMSIS/system_m2sxxx.h"

/*==============================================================================
  Private functions.
 */
static void delay(void);

/*==============================================================================
 * main() function.
 */
int main()
{
    /*
     * Initialize MSS GPIOs.
     * We assume GPIOs are configured as part of the Libero flow.
     */
    MSS_GPIO_init();
    MSS_GPIO_set_outputs( 0x55555555 );
    
    /*
     * We disable the Cortex-M3 interrupts and wait for an interrupt to occur.
     * This should result in the Cortex-M3 stopping execution.
     * Please note you will still be able to step through this code in a
     * debugger.
     */
    __disable_irq();
    __WFI();

    /*
     * Infinite loop.
     * This loop should not execute as the processor should wait forever for
     * an interrupt to occur.
     */
    MSS_GPIO_set_outputs( 0xAAAAAAAA );
    for(;;)
    {
        uint32_t gpio_pattern;
        /*
         * Decrement delay counter.
         */
        delay();
        
        /*
         * Toggle GPIO output pattern by doing an exclusive OR of all
         * pattern bits with ones.
         */
        gpio_pattern = MSS_GPIO_get_outputs();
        gpio_pattern ^= 0xFFFFFFFF;
        MSS_GPIO_set_outputs( gpio_pattern );
    }
}

/*==============================================================================
  Delay between displays of the watchdog counter value.
 */
static void delay(void)
{
    volatile uint32_t delay_count = SystemCoreClock / 128u;
    
    while(delay_count > 0u)
    {
        --delay_count;
    }
}
