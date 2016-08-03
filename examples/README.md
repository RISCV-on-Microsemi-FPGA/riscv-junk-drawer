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
$ cd openocd-riscv-flashpro-install
$ export LD_LIRARY_PATH=$(pwd)
$ ./openocd -f board/microsemi-riscv.cfg
```
