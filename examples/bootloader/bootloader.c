/*******************************************************************************
 * (c) Copyright 2016 Microsemi SoC Products Group. All rights reserved.
 *
 *  Simple boot-loader example program.
 *  This sample project is targeted at a RISC-V design running on the M2S150
 *  development board.
 *  You can program the SPI Flash from a command line program and have the
 *  boot-loader load a program from SPI Flash and jump to it.
 *  These actions are driven from a serial command line interface.
 *
 * SVN $Revision: $
 * SVN $Date: $
 */

#include "hw_platform.h"

#include "drivers/CoreGPIO/core_gpio.h"
#include "drivers/CoreUARTapb/core_uart_apb.h"
#include "drivers/CoreSPI/core_spi.h"
#include "ymodem/ymodem.h"
#include "spi_flash.h"
#include <string.h>
#include "drivers/CoreTimer/core_timer.h"

#define FLASH_SECTOR_SIZE   65536 // Sectors are 64K bytes
#define FLASH_SECTORS       128   // This is an 8MB part with 128 sectors of 64KB
#define FLASH_BLOCK_SIZE    4096  // We will use the 4K blocks for this example
#define FLASH_SEGMENT_SIZE  256   // Write segment size

#define FLASH_BLOCK_SEGMENTS ( FLASH_BLOCK_SIZE / FLASH_SEGMENT_SIZE )

static int test_flash(void);
static void mem_test(uint8_t *address);
static void rx_app_file(uint8_t *dest_address);
static void Bootloader_JumpToApplication(uint32_t stack_location, uint32_t reset_vector);
static int read_program_from_flash(uint8_t *read_buf);
static int write_program_to_flash(uint8_t *write_buf);
static int read_program_from_flash(uint8_t *read_buf);

/*
 * Base address of DDR memory where executable will be loaded.
 */
#define DDR_BASE_ADDRESS    0x80000000
 
/*
 * Delay loop down counter load value.
 */
#define DELAY_LOAD_VALUE     0x00008000

/*
 * CoreGPIO instance data.
 */
gpio_instance_t g_gpio;

volatile uint32_t g_10ms_count;
volatile uint32_t g_state;


/******************************************************************************
 * Maximum receiver buffer size.
 *****************************************************************************/
#define MAX_RX_DATA_SIZE    256

/******************************************************************************
 * PLIC instance data.
 *****************************************************************************/
plic_instance_t g_plic;

/******************************************************************************
 * CoreUARTapb instance data.
 *****************************************************************************/
extern UART_instance_t g_uart;

/******************************************************************************
 * Instruction message. This message will be transmitted over the UART to
 * HyperTerminal when the program starts.
 *****************************************************************************/
const uint8_t g_greeting_msg[] =
"\r\n\r\n\
===============================================================================\r\n\
                    Microsemi RISC-V Boot-loader v0.0.7\r\n\
===============================================================================\r\n\
";

const uint8_t g_instructions_msg[] =
"\r\n\r\n\
 Type 0 to show this menu\n\r\
 Type 1 to copy to kick-off Ymodem transfer\n\r\
 Type 2 copy program to flash \n\r\
 Type 3 copy program from flash to DDDR \n\r\
 Type 4 kick-off program in DDR \n\r\
 Type 5 to test Flash device 0\n\r\
 Type 6 to test DDR\n\r\
 Type 7 to test soft rest reloading program to LSRAM (CoreBootStrap)\n\r\
";

const uint8_t g_message[] =
"\n\r\n\r\n\r\
Simple RTG4 Dev. kit boot-loader demo (CoreBootStrap) 0.0.7\n\r\
All characters typed will be echoed back.\n\r\
Type 0 to show this menu\n\r\
Type 1 to copy to kick-off Ymodem transfer\n\r\
Type 2 copy program to flash \n\r\
Type 3 copy program from flash to DDDR \n\r\
Type 4 kick-off program in DDR \n\r\
Type 5 to test Flash device 0\n\r\
Type 6 to test DDR\n\r\
Type 7 to test soft rest reloading program to LSRAM (CoreBootStrap)\n\r\
";

/******************************************************************************
 * Timer load value. This value is calculated to result in the timer timing
 * out after after 1 second with a system clock of 24MHz and the timer
 * prescaler set to divide by 1024.
 *****************************************************************************/
#define TIMER_LOAD_VALUE    23437

/******************************************************************************
 * GPIO instance data.
 *****************************************************************************/
gpio_instance_t g_gpio;

/******************************************************************************
 * Timer 0 instance data.
 *****************************************************************************/
timer_instance_t g_timer0;
timer_instance_t g_timer1;

/*-------------------------------------------------------------------------*//**
 * main() function.
 */
int main()
{
	uint8_t rx_data[MAX_RX_DATA_SIZE];
	size_t rx_size;
	uint32_t frequency = 100;
	char message[100], wait_in_bl;

    g_10ms_count = 0;
    
    /**************************************************************************
     * Initialize the RISC-V platform level interrupt controller. 
     *************************************************************************/
    PLIC_init(&g_plic, PLIC_BASE_ADDR, PLIC_NUM_SOURCES, PLIC_NUM_PRIORITIES);
    
    /**************************************************************************
     * Initialize the CoreGPIO driver with the base address of the CoreGPIO
     * instance to use and the initial state of the outputs.
     *************************************************************************/
    GPIO_init( &g_gpio, COREGPIO_BASE_ADDR, GPIO_APB_32_BITS_BUS );

    /**************************************************************************
     * Configure the GPIOs.
     *************************************************************************/
	GPIO_set_output( &g_gpio, GPIO_0, 0x00 );
	GPIO_set_output( &g_gpio, GPIO_1, 0x00 );

    /**************************************************************************
      * Initialize CoreUARTapb with its base address, baud value, and line
      * configuration.
      *************************************************************************/
	UART_init( &g_uart, COREUARTAPB0_BASE_ADDR,\
			BAUD_VALUE_115200, (DATA_8_BITS | NO_PARITY) );


    /**************************************************************************
     * Display greeting message message.
     *************************************************************************/
	UART_polled_tx_string( &g_uart, g_greeting_msg);

    /**************************************************************************
     * Set up CoreTimer
     *************************************************************************/
#if 1     
    TMR_init(&g_timer0,
             CORETIMER0_BASE_ADDR,
             TMR_CONTINUOUS_MODE,
             PRESCALER_DIV_1024, // (83MHZ / 1024) ~ 83kHz
             810); // (83kHz / 810) ~ 10ms
           
    // In this version of the PLIC, the priorities are fixed at 1.
    // Lower numbered devices have higher priorities.
    // But this code is given as an
    // example.
    PLIC_set_priority(&g_plic, TIMER0_IRQn, 1);  

    // Enable Timer 1 & 0 Interrupt
    PLIC_enable_interrupt(&g_plic, TIMER0_IRQn);  

    // Enable the Machine-External bit in MIE
    set_csr(mie, MIP_MEIP);

    // Enable interrupts in general.
    set_csr(mstatus, MSTATUS_MIE);
    
    g_state = 1;

    // Enable the timers...
    TMR_enable_int(&g_timer0);

    // Start the timer
    TMR_start(&g_timer0);
#endif

    /**************************************************************************
     * Start the timer.
     *************************************************************************/
    TMR_start( &g_timer0 );

   	/*
	 * Check to see if boot-loader switch is set
	 */
	if (GPIO_get_inputs( &g_gpio) & 0x04)
	{
		wait_in_bl = 1;
        UART_polled_tx_string( &g_uart, "\r\n Bootloader jumper/switch set to stay in bootloader.\r\n");
        UART_polled_tx_string( &g_uart, g_instructions_msg);
	}
	else
	{
		wait_in_bl = 0;
        UART_polled_tx_string( &g_uart, "\r\n Bootloader jumper/switch set to start application.\r\n");
	}

    while(wait_in_bl == 1)
    {
         /**********************************************************************
         * Read data received by the UART.
         *********************************************************************/
        rx_size = UART_get_rx( &g_uart, rx_data, sizeof(rx_data) );


        /**********************************************************************
         * Echo back data received, if any.
         *********************************************************************/
        if ( rx_size > 0 )
        {
            UART_send( &g_uart, rx_data, rx_size );

            switch(rx_data[0])
            {
                case '0':
                    UART_polled_tx_string( &g_uart, g_instructions_msg);
                    break;
                case '1':
                    rx_app_file((uint8_t *)DDR_BASE_ADDRESS);
                    break;
                case '2':
                    write_program_to_flash((uint8_t *)DDR_BASE_ADDRESS);
                    break;
                case '3':
                    read_program_from_flash((uint8_t *)DDR_BASE_ADDRESS);
                    break;
                case '4':
                	/* Populate with stack and reset vector address i.e. The first two words of the program */
                    Bootloader_JumpToApplication((uint32_t)0x60000000, (uint32_t)0x60000004);
                    break;
                case '5':
                    /* Works- but needed to blow open Libero "RTG4 DDR Memory controller with initialization"
                     * and edit with parameters taken from design on web site- see ABC program below */
                    test_flash();
                    break;
                case '6':
                    /*
                     * Sanity check on DDR working
                     */
                    mem_test((uint8_t *)DDR_BASE_ADDRESS);
                    break;
                case '7':
					/* test soft reset- with CoreBootStrap */
                	/* wipe reset vector */
                	/* choose this option */
                	/* system should reboot correctly */
//<CJ>                	__disable_irq();
//<CJ>                	*acr = 0x04;  						/*; make sure ITCM mapped to 0x10000000 only */
//<CJ>                	asm ("DSB  ");                      /*; ensure that store completes before      */
                	                                    /*; executing the next instruction  		*/
//<CJ>                	asm ("ISB  ");                      /*; executing synchronization instruction   */
                	/*Set reset vector in LSRAM tyo zero */
//<CJ>                	*(uint32_t *)0x00000004 = 0;
//<CJ>                	NVIC_SystemReset();		/* RESET the system- program will be loaded by CoreBootStrap */
			break;
            }
        }
    }
    UART_polled_tx_string( &g_uart, "\r\n Loading application from SPI flash into DDR memory.\r\n");
    read_program_from_flash((uint8_t *)DDR_BASE_ADDRESS);
    UART_polled_tx_string( &g_uart, "\r\n Executing application in DDR memory.\r\n");
    Bootloader_JumpToApplication(0x70000000, 0x70000004);
    /* will never reach here! */
    while(1);
    return 0;
}

/*
 *  Test flash on RTG4
 */
static int test_flash(void)
{
    uint8_t write_buffer[FLASH_SEGMENT_SIZE];
    uint8_t read_buffer[FLASH_SEGMENT_SIZE];
    uint16_t status;
    int flash_address = 0;
    int count = 0;
    spi_flash_status_t result;
    struct device_Info DevInfo;

    spi_flash_init();

    spi_flash_control_hw( SPI_FLASH_RESET, 0, &status );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                          count * FLASH_SECTOR_SIZE,
                                           &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );

    /*--------------------------------------------------------------------------
     * First fetch status register. First byte in low 8 bits, second byte in
    * upper 8 bits.
     */
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );




    /*--------------------------------------------------------------------------
     * Fetch protection register value for each of the 128 sectors.
     * After power up these should all read as 0xFF
     */
   for( count = 0; count != 128; ++count )
   {
    result = spi_flash_control_hw( SPI_FLASH_GET_PROTECT,
                                      count * FLASH_SECTOR_SIZE,
                                       &read_buffer[count] );
   }

   //device D
   result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );

   /*--------------------------------------------------------------------------
    * Show sector protection in action by:
    *   - unprotecting the first sector
    *   - erasing the sector
    *   - writing some data to the first 256 bytes
    *   - protecting the first sector
    *   - erasing the first sector
    *   - reading back the first 256 bytes of the first sector
    *   - unprotecting the first sector
    *   - erasing the sector
    *   - reading back the first 256 bytes of the first sector
    *
    * The first read should still show the written data in place as the erase
    * will fail. the second read should show all 0xFFs. Step through the code
    * in debug mode and examine the read buffer after the read operations to
    * see this.
    */
   result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
   //device D   works
   result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D-- now working
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    memset( write_buffer, count, FLASH_SEGMENT_SIZE );
    strcpy( (char *)write_buffer, "Microsemi FLASH test" );
    spi_flash_write( flash_address, write_buffer, FLASH_SEGMENT_SIZE );
       //device D --
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_SECTOR_PROTECT, flash_address, NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    spi_flash_read ( flash_address, read_buffer, FLASH_SEGMENT_SIZE);
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    spi_flash_read ( flash_address, read_buffer, FLASH_SEGMENT_SIZE );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    /*--------------------------------------------------------------------------
     * Read the protection registers again so you can see that the first sector
     * is unprotected now.
     */
    for( count = 0; count != 128; ++count )
   {
    spi_flash_control_hw( SPI_FLASH_GET_PROTECT, count * FLASH_SECTOR_SIZE,
                             &write_buffer[count] );
   }
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    /*--------------------------------------------------------------------------
     * Write something to all 32768 blocks of 256 bytes in the 8MB FLASH.
     */
   for( count = 0; count != 1000 /*32768*/; ++count )
   {
        /*----------------------------------------------------------------------
         * Vary the fill for each chunk of 256 bytes
         */
        memset( write_buffer, count, FLASH_SEGMENT_SIZE );
        strcpy( (char *)write_buffer, "Microsemi FLASH test" );
        /*----------------------------------------------------------------------
         * at the start of each sector we need to make sure it is unprotected
         * so we can erase blocks within it. The spi_flash_write() function
         * unprotects the sector as well but we need to start erasing before the
         * first write takes place.
         */
        if(0 == (flash_address % FLASH_SECTOR_SIZE))
        {
            result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
        }
        /*----------------------------------------------------------------------
         * At the start of each 4K block we issue an erase so that we are then
         * free to write anything we want to the block. If we don't do this the
         * write may fail as we can only effectively turn 1s to 0s when we
         * write. For example if we have an erased location with 0xFF in it and
         * we write 0xAA to it first and then later on write 0x55, the resulting
         * value is 0x00...
         */
        if(0 == (flash_address % FLASH_BLOCK_SIZE))
        {
            result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
        }
        /*----------------------------------------------------------------------
         * Write our values to the FLASH, read them back and compare.
         * Placing a breakpoint on the while statement below will allow
         * you break on any failures.
         */
        spi_flash_write( flash_address, write_buffer, FLASH_SEGMENT_SIZE );
        spi_flash_read ( flash_address, read_buffer, FLASH_SEGMENT_SIZE );
        if( memcmp( write_buffer, read_buffer, FLASH_SEGMENT_SIZE ) )
        {
            while(1) // Breakpoint here will trap write faults
            {
                   result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                              count * FLASH_SECTOR_SIZE,
                                                               &DevInfo );
                   spi_flash_control_hw( SPI_FLASH_RESET, 0, &status );

            }

        }

        flash_address += FLASH_SEGMENT_SIZE; /* Step to the next 256 byte chunk */
    }
   /*--------------------------------------------------------------------------
    * One last look at the protection registers which should all be 0 now
    */
   for( count = 0; count != 128; ++count )
   {
    spi_flash_control_hw( SPI_FLASH_GET_PROTECT, count * FLASH_SECTOR_SIZE,
                            &write_buffer[count] );
   }

   UART_send( &g_uart, "Flash test success\n\r", strlen("Flash test success\n\r") );

   return(0);
}



/*
 *  Write to flash on RTG4
 */
static int write_program_to_flash(uint8_t *write_buf)
{
    uint8_t write_buffer[FLASH_SEGMENT_SIZE];
    uint8_t read_buffer[FLASH_SEGMENT_SIZE];
    uint16_t status;
    int flash_address = 0;
    int count = 0;
    spi_flash_status_t result;
    struct device_Info DevInfo;

    spi_flash_init();

    spi_flash_control_hw( SPI_FLASH_RESET, 0, &status );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                          count * FLASH_SECTOR_SIZE,
                                           &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );

    /*--------------------------------------------------------------------------
     * First fetch status register. First byte in low 8 bits, second byte in
    * upper 8 bits.
     */
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );




    /*--------------------------------------------------------------------------
     * Fetch protection register value for each of the 128 sectors.
     * After power up these should all read as 0xFF
     */
   for( count = 0; count != 128; ++count )
   {
    result = spi_flash_control_hw( SPI_FLASH_GET_PROTECT,
                                      count * FLASH_SECTOR_SIZE,
                                       &read_buffer[count] );
   }

   //device D
   result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );

   /*--------------------------------------------------------------------------
    * Show sector protection in action by:
    *   - unprotecting the first sector
    *   - erasing the sector
    *   - writing some data to the first 256 bytes
    *   - protecting the first sector
    *   - erasing the first sector
    *   - reading back the first 256 bytes of the first sector
    *   - unprotecting the first sector
    *   - erasing the sector
    *   - reading back the first 256 bytes of the first sector
    *
    * The first read should still show the written data in place as the erase
    * will fail. the second read should show all 0xFFs. Step through the code
    * in debug mode and examine the read buffer after the read operations to
    * see this.
    */
   result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
   //device D   works
   result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D-- now working
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    memset( write_buffer, count, FLASH_SEGMENT_SIZE );
    strcpy( (char *)write_buffer, "Microsemi FLASH test" );
    spi_flash_write( flash_address, write_buffer, FLASH_SEGMENT_SIZE );
       //device D --
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_SECTOR_PROTECT, flash_address, NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    spi_flash_read ( flash_address, read_buffer, FLASH_SEGMENT_SIZE);
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );

    spi_flash_read ( flash_address, read_buffer, FLASH_SEGMENT_SIZE );
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    /*--------------------------------------------------------------------------
     * Read the protection registers again so you can see that the first sector
     * is unprotected now.
     */
    for( count = 0; count != 128; ++count )
   {
    spi_flash_control_hw( SPI_FLASH_GET_PROTECT, count * FLASH_SECTOR_SIZE,
                             &write_buffer[count] );
   }
       //device D
       result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                                  count * FLASH_SECTOR_SIZE,
                                                   &DevInfo );
    /*--------------------------------------------------------------------------
     * Write something to all 32768 blocks of 256 bytes in the 8MB FLASH.
     */
   for( count = 0; count != ((32*1024)/256) /*32768*/; ++count )
   {
        /*----------------------------------------------------------------------
         * Vary the fill for each chunk of 256 bytes
         */
        memset( write_buffer, count, FLASH_SEGMENT_SIZE );
        strcpy( (char *)write_buffer, "Microsemi FLASH test" );
        /*----------------------------------------------------------------------
         * at the start of each sector we need to make sure it is unprotected
         * so we can erase blocks within it. The spi_flash_write() function
         * unprotects the sector as well but we need to start erasing before the
         * first write takes place.
         */
        if(0 == (flash_address % FLASH_SECTOR_SIZE))
        {
            result = spi_flash_control_hw( SPI_FLASH_SECTOR_UNPROTECT, flash_address, NULL );
        }
        /*----------------------------------------------------------------------
         * At the start of each 4K block we issue an erase so that we are then
         * free to write anything we want to the block. If we don't do this the
         * write may fail as we can only effectively turn 1s to 0s when we
         * write. For example if we have an erased location with 0xFF in it and
         * we write 0xAA to it first and then later on write 0x55, the resulting
         * value is 0x00...
         */
        if(0 == (flash_address % FLASH_BLOCK_SIZE))
        {
            result = spi_flash_control_hw( SPI_FLASH_4KBLOCK_ERASE, flash_address , NULL );
        }
        /*----------------------------------------------------------------------
         * Write our values to the FLASH, read them back and compare.
         * Placing a breakpoint on the while statement below will allow
         * you break on any failures.
         */
        spi_flash_write( flash_address, write_buf, FLASH_SEGMENT_SIZE );

        spi_flash_read ( flash_address, read_buffer, FLASH_SEGMENT_SIZE );
        if( memcmp( write_buf, read_buffer, FLASH_SEGMENT_SIZE ) )
        {
            while(1) // Breakpoint here will trap write faults
            {

            }

        }
        write_buf += FLASH_SEGMENT_SIZE;
        flash_address += FLASH_SEGMENT_SIZE; /* Step to the next 256 byte chunk */
    }
   /*--------------------------------------------------------------------------
    * One last look at the protection registers which should all be 0 now
    */
   for( count = 0; count != 128; ++count )
   {
    spi_flash_control_hw( SPI_FLASH_GET_PROTECT, count * FLASH_SECTOR_SIZE,
                            &write_buffer[count] );
   }

   UART_send( &g_uart, "Flash write success\n\r", strlen("Flash write success\n\r") );

   return(0);
}


/**
 *  Read from flash on RTG4
 */
static int read_program_from_flash(uint8_t *read_buf)
{
    uint8_t write_buffer[FLASH_SEGMENT_SIZE];
    uint8_t read_buffer[FLASH_SEGMENT_SIZE];
    uint16_t status;
    int flash_address = 0;
    int count = 0;
    spi_flash_status_t result;
    struct device_Info DevInfo;

    spi_flash_init();

    spi_flash_control_hw( SPI_FLASH_RESET, 0, &status );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                          count * FLASH_SECTOR_SIZE,
                                           &DevInfo );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );

    /*--------------------------------------------------------------------------
     * First fetch status register. First byte in low 8 bits, second byte in
    * upper 8 bits.
     */
    result = spi_flash_control_hw( SPI_FLASH_GET_STATUS, 0, &status );

    result = spi_flash_control_hw( SPI_FLASH_READ_DEVICE_ID,
                                              count * FLASH_SECTOR_SIZE,
                                               &DevInfo );


    /*--------------------------------------------------------------------------
     * Write something to all 32768 blocks of 256 bytes in the 8MB FLASH.
     */
   for( count = 0; count != ((32*1024)/256); ++count )
   {

        /*----------------------------------------------------------------------
         * Write our values to the FLASH, read them back and compare.
         * Placing a breakpoint on the while statement below will allow
         * you break on any failures.
         */

        spi_flash_read ( flash_address, read_buf, FLASH_SEGMENT_SIZE );
        read_buf += FLASH_SEGMENT_SIZE;

        flash_address += FLASH_SEGMENT_SIZE; /* Step to the next 256 byte chunk */
    }

   UART_send( &g_uart, "Flash read success\n\r", strlen("Flash read success\n\r") );

   return(0);
}

/*
 * Simple sanity check
 */
static void mem_test(uint8_t *address)
{
    volatile uint8_t value=2;
    volatile uint32_t value32=3;
    *address = 1;
    value = *address;
    value32 = (uint32_t)*address;
    if((value32 == value) &&(value == 1))
    	UART_send( &g_uart, "Read/Write success\n\r", strlen("Read/Write success\n\r") );
    else
    	UART_send( &g_uart, "Read/Write fail\n\r", strlen("Read/Write fail\n\r") );
}


/*
 * Put image received via ymodem into memory
 */
static void rx_app_file(uint8_t *dest_address)
{
    uint8_t     received;
    uint8_t *g_bin_base = (uint8_t *)dest_address;
    uint32_t g_rx_size = 1024 * 16;
    received = ymodem_receive(g_bin_base, g_rx_size);
}

/*------------------------------------------------------------------------------
 * Count the number of elapsed milliseconds (SysTick_Handler is called every
 * 10mS so the resolution will be 10ms). Rolls over every 49 days or so...
 *
 * Should be safe to read g_10ms_count from elsewhere.
 */
void SysTick_Handler( void )
{
    uint32_t gpio_pattern;
    static uint8_t count;
    /*
     * Toggle GPIO output pattern by doing an exclusive OR of all
     * pattern bits with ones.
     */
    if(count++>=50)
    {
        gpio_pattern = GPIO_get_outputs( &g_gpio );
        gpio_pattern ^= 0x00000002;
        GPIO_set_outputs( &g_gpio, gpio_pattern );
        count=0;
    }

    g_10ms_count += 10;

     /*
      * For neatness, if we roll over, reset cleanly back to 0 so the count
      * always goes up in proper 10s.
      */
    if(g_10ms_count < 10)
        g_10ms_count = 0;
}

/***************************************************************************//**
* CoreTimer 0 Interrupt Handler.
*/
void Timer0_IRQHandler() {
    uint32_t stable;
    uint32_t gpout;
  
    stable = GPIO_get_inputs(&g_gpio);
    gpout = ~stable & 0x000000F0;
  
    g_state = g_state << 1;
    if (g_state > 4) {
        g_state = 0x01;
    }
    gpout = gpout | g_state;
   
  
    GPIO_set_outputs(&g_gpio, gpout);
 
    g_10ms_count += 10;

     /*
      * For neatness, if we roll over, reset cleanly back to 0 so the count
      * always goes up in proper 10s.
      */
    if(g_10ms_count < 10)
        g_10ms_count = 0;
  
    TMR_clear_int(&g_timer0);
}

/******************************************************************************
 * RISC-V interrupt handler for external interrupts.
 *****************************************************************************/
/*Entry Point for PLIC Interrupt Handler*/
void handle_m_ext_interrupt(){
  plic_source int_num  = PLIC_claim_interrupt(&g_plic);
  switch(int_num) {
  case (0):
    break;
  case (External_30_IRQn): 
    Timer0_IRQHandler();
    break;
  case (External_31_IRQn): 
    break;
  default: 
    _exit(10 + (uintptr_t) int_num);
  }
  PLIC_complete_interrupt(&g_plic, int_num);
}

/*
 * Call this function if you want to switch to another program
 * de-init any loaded drivers before calling this function
 */
//volatile uint32_t cj_debug;
static void Bootloader_JumpToApplication(uint32_t stack_location, uint32_t reset_vector)
{
//    cj_debug = 0x12345678;
//    __asm volatile("lui t0,0x80000");
//    __asm volatile("csrw mepc,t0");
    __asm volatile("lui ra,0x80000");
    

//<CJ>    __disable_irq(); /* ro, r1 will not be disturbed */
    /*Load main stack pointer with application stack pointer initial value,
      stored at first location of application area */
    //asm volatile("ldr r0, =0x30000000");
//<CJ>    asm volatile("ldr r0, [r0]");
//<CJ>    asm volatile("mov sp, r0");

    /*Load program counter with application reset vector address, located at
      second word of application area. */
//<CJ>    asm volatile("mov r3, r1");
//<CJ>    asm volatile("mov r0, #0");  /*  ; no arguments  */
//<CJ>    asm volatile("mov r1, #0");  /*  ; no argv either */
    //asm volatile("ldr r3, =0x30000004");
//<CJ>    asm volatile("ldr r3, [r3]");
//<CJ>    asm volatile("mov pc, r3");
    /*User application execution should now start and never return here.... */
}
