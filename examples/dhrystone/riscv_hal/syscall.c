#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include "encoding.h"

#include "shared.h"
#include "coregpio_regs.h"

UART_instance_t g_uart;

#undef errno
int errno;

#define __isatty(fd) ((fd) == STDOUT_FILENO || (fd) == STDERR_FILENO)

char *__env[1] = { 0 };
char **environ = __env;

volatile uint64_t tohost __attribute__((aligned(64)));
volatile uint64_t fromhost __attribute__((aligned(64)));

void _exit(int code)
{
  volatile uint32_t* leds = (uint32_t*) (COREGPIO_OUT_BASE_ADDR + GPIO_OUT0_REG_OFFSET);
  // This happens to just light up the red LEDS.
  *leds = (0x30); 
  printf ("\nProgam has exited with code %x\n", code);
  while (1);
}

void *sbrk(ptrdiff_t incr)
{
  extern char _heap_begin[];
  extern char _heap_end[];
  static char *curbrk;
  
  if (curbrk == 0)
    curbrk = _heap_begin;

  if ((curbrk + incr < _heap_begin) || (curbrk + incr > _heap_end))
    return NULL - 1;

  curbrk += incr;
  return curbrk - incr;
}

static int stub(int err)
{
  errno = err;
  return -1;
}

int open(const char* name, int flags, int mode)
{
  return stub(ENOENT);
}

int close(int fd)
{
  return stub(EBADF);
}

int fstat(int fd, struct stat *st)
{
  if (__isatty(fd)) {
    st->st_mode = S_IFCHR;
    return 0;
  }

  return stub(EBADF);
}

int isatty(int fd)
{
  if (__isatty(fd))
    return 1;

  errno = EBADF;
  return 0;
}

off_t lseek(int fd, off_t ptr, int dir)
{
  if (__isatty(fd))
    return 0;

  return stub(EBADF);
}

ssize_t read(int fd, void* ptr, size_t len)
{
  if (isatty(fd))
    return UART_get_rx(&g_uart,
                       (uint8_t*) ptr,
                       len);

  return stub(EBADF);
}

ssize_t write(int fd, const void* ptr, size_t len)
{

  const char * current;
  const char * head = (const char *) ptr;
  
  if (isatty(fd)) {
     
    for (current = head; current < (head + len) ; current++){
      UART_send(&g_uart,
                current,
                1);
      if ( *current == '\n'){
        UART_send(&g_uart,
                  "\r", 1);
      }
    }
    
    return len;
  }

  return stub(EBADF);
}
