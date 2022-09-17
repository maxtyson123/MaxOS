
# Max Os

Max os following: https://wiki.osdev.org/Creating_an_Operating_System

Made in C and asm

## Run Locally

Clone the project

```bash
  git clone https://github.com/maxtyson123/max-os
```

(You may need a copy of https://github.com/lordmilko/i686-elf-tools)

Go to the project directory

```bash
  cd my-project
```

Make Files and OS

```bash
i686-elf-as boot.s -o boot.o
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
```

Run Os 

```bash
  qemu-system-i386 -kernel myos.bin
```


