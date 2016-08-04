# riscv-junk-drawer
Various helping bits and pieces for using RISC-V on Microsemi FPGA.

### examples
The examples folder contains example RISC-V software projects.

### IP-packages
The IP-packages folder contains RISC-V processor IP packages for the Libero IP
Catalog. You will need one of these IP packages to easily include a RISC-V
processor into a Microsemi FPGA design.

### M3-disable
The M3-disable folder contains the SoftConsole projects used to create the
SmartFusion2 Cortex-M3 firmware used to configure the SmartFusion2 SoC FPGA
before handing over control to a RISC-V processor in the FPGA fabric.
Multiple versions of this firmware exists to support the various Microsemi
development boards.
