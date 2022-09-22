
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
 nasm -f elf32 loader.s
 ld -T link.ld -melf_i386 loader.o -o kernel.elf
 cp kernel.elf iso/boot/       
```

Run Os 

```bash
     bochs -f bochsrc.txt -q
```


