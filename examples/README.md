Example Software Projects
=========================

This folder contains example software projects. Each project is self contained.

Tools for building these examples
---------------------------------

These projects can be built using SiFive's Freedom-e-SDK:  http://github.com/sifive/freedom-e-sdk

Alternatively, a build of the toolchain is available from the following link: ftp://ftp.actel.com/outgoing/RISC-V/toolchain/latest/toolchain.tar.gz

### Debugging using FlashPro5
A build of the version of openocd allowing debugging using FlashPro5 is available from the following link: ftp://ftp.actel.com/outgoing/RISC-V/toolchain/latest/openocd-riscv-flashpro-install.tar.gz

Once extracted you will need to export LD_LIRARY_PATH to point at the openocd-riscv-flashpro-install/bin directory. e.g:
```sh
$ tar -xvf openocd-riscv-flashpro-install.tar.gz
$ cd openocd/bin
$ export LD_LIRARY_PATH=$(pwd)
$ ./openocd -f board/microsemi-riscv.cfg
```

Once openocd is running, start GDB in another terminal.
```sh
$ riscv64-unknown-elf-gdb <my_executable>
```

In GDB, connect to the target, load the executable to debug and run.
```sh
(gdb) target remote localhost:3333
Remote debugging using localhost:3333
0x60000ff8 in ?? ()
(gdb) load
Loading section .text, size 0x5110 lma 0x80000000
Loading section .eh_frame, size 0x4a8 lma 0x80005110
Loading section .rodata, size 0x894 lma 0x800055b8
Loading section .sdata, size 0x4 lma 0x80040000
Start address 0x80000000, load size 24144
Transfer rate: 71 KB/sec, 2682 bytes/write.
(gdb) b main
Breakpoint 1 at 0x800003c4
(gdb) c
Continuing.

Breakpoint 1, 0x800003c4 in main ()
(gdb) 
```
