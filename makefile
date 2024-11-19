default:
	arm-none-eabi-gcc -mcpu=cortex-a7 -fpic -ffreestanding -c boot.S -o objects/boot.o
	arm-none-eabi-gcc -mcpu=cortex-a7 -fpic -ffreestanding -std=gnu99 -c kernel.c -o objects/kernel.o -O2 -Wall -Wextra
	arm-none-eabi-gcc -T linker.ld -o objects/myos.elf -ffreestanding -O2 -nostdlib objects/boot.o objects/kernel.o
	arm-none-eabi-objcopy -O binary objects/myos.elf kernel7.img