# IP Packages
This folder contains RISC-V IP packages that can be imported into the Libero IP Catalog.
Importing these IP packages will allow adding a RISC-V processor into your FPGA design by selecting the processor of your choice from the IP Catalog in Libero.

### Importing an IP package into Libero
IP packages are imported into Libero using the File->Import->Others menu item and selecting a .cpz IP package file.
For example importing the Actel_DirectCore_CORERISCV_AXI4_1.0.100.cpz file will result in the CoreRISCV_AXI4 version 1.0.100 becoming availalbe from the processors section of the Libero IP Catalog.

### Actel_DirectCore_CORERISCV_AXI4_1.0.100.cpz
This IP package contains CoreRISCV_AXI4 supporting the RV32IM instruction set. Connection to cached memory and peripherals is thropugh two separate AXI4 interfaces.

### Actel_DirectCore_COREJTAGDEBUG_1.0.101.cpz
This IP packages contains the digital logic required to connect to a soft processor's JTAG debug interface through the FPGA's external JTAG interface. This allows debugging software executing on the soft processor using the same JTAG interface used to program the FPGA. This IP block is Microsemi FPGA specific.
This IP package provides a simple solution for debugging software using [SoftConsole](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole) and [FlashPro5](http://www.microsemi.com/products/fpga-soc/design-resources/programming/flashpro#hardware) FPGA programming hardware.

### SiFive_SiFive_SiFiveE31Coreplex_1.0.8.cpz
This IP package contains [SiFive's](https://www.sifive.com/) Coreplex E31. This is an RV32IM instruction set implementation.

### User_GlueLogic_AXI_GLUE_LOGIC_1.0.7.cpz
This IP package contains some user glue logic for connecting CoreRISCV_AXI4 or a Coreplex E31 bus interfaces in a Libero SmartDesign project.

