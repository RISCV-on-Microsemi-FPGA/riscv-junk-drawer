LINKER_SCRIPT := ./riscv_hal/link.lds
#LINKER_SCRIPT := all-ram.lds
#LINKER_SCRIPT := ./riscv_hal/all-ram-2.lds

ASM_SRCS :=
C_SRCS := ./riscv_hal/init.c dhry_1.c dhry_2.c dhry_stubs.c dhry_printf.c
RISCV_HAL_ASM_SRCS := ./riscv_hal/entry.S
RISCV_HAL_C_SRCS := ./riscv_hal/syscall.c
HEADERS := ./riscv_hal/encoding.h dhry.h

DHRYSTONE_CFLAGS := -DTIME -fno-inline -Wno-implicit

C_SRCS += ./hal/hw_reg_access.c
C_SRCS += ./drivers/CoreGPIO/core_gpio.c
C_SRCS += ./drivers/CoreUARTapb/core_uart_apb.c
C_SRCS += ./drivers/CoreTimer/core_timer.c
C_SRCS += ./drivers_sifive/plic.c

INCLUDES += -I.
INCLUDES += -I./riscv_hal
INCLUDES += -I./drivers/CoreGPIO
INCLUDES += -I./drivers/CoreUARTapb
INCLUDES += -I./drivers/CoreTimer
INCLUDES += -I./drivers_sifive/
INCLUDES += -I./hal

XLEN ?= 32
CC := riscv$(XLEN)-unknown-elf-gcc
CFLAGS := $(DHRYSTONE_CFLAGS) -O2 -march=RV$(XLEN)IM -msoft-float -fno-common -fno-builtin-printf -fno-toplevel-reorder $(INCLUDES)
LDFLAGS := -T $(LINKER_SCRIPT) -L./riscv_hal -nostdlib -nostartfiles -lc -lgcc
RUN := spike --isa=RV$(XLEN)IM

ASM_OBJS := $(patsubst %.S,%.o,$(ASM_SRCS))
RISCV_HAL_ASM_OBJS := $(patsubst %.S,%.o,$(RISCV_HAL_ASM_SRCS))
C_OBJS := $(patsubst %.c,%.o,$(C_SRCS))
RISCV_HAL_C_OBJS := $(patsubst %.c,%.o,$(RISCV_HAL_C_SRCS))

dhrystone: $(ASM_OBJS) $(RISCV_HAL_ASM_OBJS) $(C_OBJS) $(RISCV_HAL_C_OBJS) $(LINKER_SCRIPT)
	$(CC) $(CFLAGS) $(ASM_OBJS) $(C_OBJS) -o $@ $(LDFLAGS)

$(ASM_OBJS): %.o: %.S $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(RISCV_HAL_ASM_OBJS): %.o: %.S $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(C_OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(RISCV_HAL_C_OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

ELF_TO_IHEX = objcopy -O ihex --set-start 0

dhrystone.ihx: dhrystone
	$(ELF_TO_IHEX) dhrystone dhrystone.tmp
	@echo "Please manually edit dhrystone.tmp to remove the first line and the .data section at the end. Then save as dhyrstone.ihx"

clean:
	rm -f dhrystone *.o ./riscv_hal/*.o

.PHONY: clean
