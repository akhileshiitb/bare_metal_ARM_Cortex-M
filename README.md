# bare_metal_ARM_Cortex-M
Bare metal code to checkout ARM cortex M internal working. Basic startup, exception handling etc. 

We are using QEMU with machine: -machine lm3s6965evb   is it TI device with M4 processor with 64KB SRAM

Memory map: 
FLASH= 0x0
RAM = 0x20000000 + 64KB
