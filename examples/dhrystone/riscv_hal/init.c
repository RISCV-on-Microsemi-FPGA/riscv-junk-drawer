#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "shared.h"

uintptr_t handle_trap(uintptr_t cause, uintptr_t epc)
{
  write(1, "trap\n", 5);
  _exit(1 + cause);
  return epc;
}

void _init(void)
{
  extern char _data;
  extern char _data_end;
  memset(&_data, 0, &_data_end - &_data);

  UART_init( &g_uart, COREUARTAPB0_BASE_ADDR, BAUD_VALUE_115200, (DATA_8_BITS | NO_PARITY) );

  extern int main(int, char**);
  const char *argv0 = "hello";
  char *argv[] = {(char *)argv0, NULL, NULL};

  _exit(main(1, argv));
}
