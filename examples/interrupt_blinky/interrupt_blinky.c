#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "hw_platform.h"
#include "riscv_hal.h"
#include "core_gpio.h"
#include "core_timer.h"
#include "core_uart_apb.h"

extern UART_instance_t g_uart;

const char * g_hello_msg = "\r\nInterrupt Blinky Example\r\n";
  
/******************************************************************************
 * GPIO instance data.
 *****************************************************************************/

gpio_instance_t g_gpio0;
gpio_instance_t g_gpio1;

/******************************************************************************
 * CoreTimer instance data.
 *****************************************************************************/

timer_instance_t g_timer0;
timer_instance_t g_timer1;

/******************************************************************************
 * Global state counter.
 *****************************************************************************/

uint32_t g_state;
uint8_t g_done;

/*Core Timer 0 Interrupt Handler*/
void External_30_IRQHandler() {
  uint32_t stable;
  uint32_t gpout;
  
  stable = GPIO_get_inputs(&g_gpio0);
  gpout = ~stable & 0x000000F0;
  
  g_state = g_state << 1;
  if (g_state > 4) {
    g_state = 0x01;
  }
  gpout = gpout | g_state;
   
  
  GPIO_set_outputs(&g_gpio1, gpout);
  
  TMR_clear_int(&g_timer0);
}

/*Core Timer 1 Interrupt Handler*/
void interrupt_handler_31() {
  volatile uint8_t * done = &g_done;

  if (*done){
    TMR_stop(&g_timer1);
  } else {
    g_state = g_state + 1;
    GPIO_set_outputs(&g_gpio1, g_state << 8);
  }
  
  TMR_clear_int(&g_timer1);
  
}


int main(int argc, char **argv)
{
  uint8_t rx_char;
  uint8_t rx_count;
  uint32_t switches;

  PLIC_init();
  
  GPIO_init(&g_gpio0, COREGPIO_IN_BASE_ADDR, GPIO_APB_32_BITS_BUS);
  GPIO_init(&g_gpio1, COREGPIO_OUT_BASE_ADDR, GPIO_APB_32_BITS_BUS);

  UART_polled_tx_string(&g_uart, g_hello_msg);
  
  /**************************************************************************
   * Set up CoreTimer
   *************************************************************************/
  TMR_init(&g_timer0,
           CORETIMER0_BASE_ADDR,
           TMR_CONTINUOUS_MODE,
           PRESCALER_DIV_1024, // (50MHZ / 1024) ~ 50kHz
           50000); // (50kHz / 50000) ~ 1sec
           
  // In this version of the PLIC, the priorities are fixed at 1.
  // Lower numbered devices have higher priorities.
  // But this code is given as an
  // example.
    PLIC_SetPriority(External_30_IRQn, 1);

  // Enable Timer 1 & 0 Interrupt
    PLIC_EnableIRQ(External_30_IRQn);

  // Enable the Machine-External bit in MIE
//  set_csr(mie, MIP_MEIP);

  // Enable interrupts in general.
//  set_csr(mstatus, MSTATUS_MIE);

    __enable_irq();

  // Enable the timers...
  TMR_enable_int(&g_timer0);

  g_state = 0x01;
  
  // Start the timer
  TMR_start(&g_timer0);


  /**************************************************************************
   * Loop
   *************************************************************************/
  do {
    switches = GPIO_get_inputs(&g_gpio0);
	
    rx_count = UART_get_rx(&g_uart, &rx_char, 1);
    if (rx_count > 0) {
      UART_send(&g_uart, &rx_char, 1);
    }

    GPIO_set_outputs(&g_gpio1, (~switches & 0x000000F0) | g_state);
  } while (1);
  
  
  return 0;
}

