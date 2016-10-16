#-------------------------------------------------------------------------------
# Select linker you wish to use.
# link.lds:		Builds your program to execute from SmartFusion eNVM.
# all-ram.lds:	Builds a program that can be loaded to memory with the debugger.
#
#LINKER_SCRIPT := ./link.lds
LINKER_SCRIPT := ./all-ram.lds

C_DEFINES = -DUSE_PLIC

TARGET = interrupt_blinky

ASM_SRCS += ./riscv_hal/entry.S

C_SRCS += ./hal/hw_reg_access.c
C_SRCS += ./riscv_hal/init.c
C_SRCS += ./riscv_hal/syscall.c
C_SRCS += ./riscv_hal/riscv_hal.c
C_SRCS += ./riscv_hal/riscv_hal_stubs.c
C_SRCS += ./drivers/CoreGPIO/core_gpio.c
C_SRCS += ./drivers/CoreUARTapb/core_uart_apb.c
C_SRCS += ./drivers/CoreTimer/core_timer.c

HEADERS += ./hal/*.h
HEADERS += ./riscv_hal/*.h
HEADERS += ./drivers/*/*.h
HEADERS += ./*.h

C_SRCS += $(TARGET).c
INCLUDES += -I.
INCLUDES += -I./riscv_hal
INCLUDES += -I./drivers/CoreGPIO
INCLUDES += -I./drivers/CoreUARTapb
INCLUDES += -I./drivers/CoreTimer
INCLUDES += -I./hal

ASM_OBJS := $(patsubst %.S,%.o,$(ASM_SRCS))
C_OBJS := $(patsubst %.c,%.o,$(C_SRCS))

CC := riscv32-unknown-elf-gcc
CFLAGS := -Og -g -march=RV32IM -fno-common -fno-jump-tables $(INCLUDES) $(C_DEFINES)
LDFLAGS := -T $(LINKER_SCRIPT) -nostdlib -nostartfiles -lc -lgcc

$(TARGET): $(ASM_OBJS) $(C_OBJS) $(LINKER_SCRIPT)
	$(CC) $(CFLAGS) $(C_OBJS) -o $@ $(LDFLAGS)

$(ASM_OBJS): %.o: %.S $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(C_OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

ELF_TO_IHEX = objcopy -O ihex --set-start 0

$(TARGET).ihx: $(TARGET)
	$(ELF_TO_IHEX) $(TARGET) $(TARGET).tmp
	tail -n +2 $(TARGET).tmp > $(TARGET).ihx

default: $(TARGET).ihx

all: default

clean:
	rm -f $(TARGET) *.o *.ihx ./*.o ./drivers/*/*.o ./hal/*.o ./riscv_hal/*.o

.PHONY: clean all default
