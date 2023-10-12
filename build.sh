# compile C source 
arm-none-eabi-gcc -mthumb  -march=armv7e-m  -g -S -x c main.c -o main.s

# Assemble
arm-none-eabi-as -mthumb -march=armv7e-m main.s  -o main.obj
arm-none-eabi-as -mthumb -march=armv7e-m startup.s  -o startup.obj

# linking 
arm-none-eabi-ld -o image.elf --thumb-entry=_start -Ttext 0x0 -Tdata 0x20000000 -Map image.map startup.obj  main.obj


# objdump .. for binary creation 


# objdump to get full disassembly
arm-none-eabi-objdump  --disassemble-all image.elf  > image.dis 
