.PHONY: default
default: build;

TARGET=i686-elf

GCC_EXEC ?= toolchain/cross_compiler/cross/bin/$(TARGET)-gcc
GCC_PARAMS = -ffreestanding -fno-exceptions -fno-rtti -nostdlib -Wall -Wextra

AS_EXEC ?= toolchain/cross_compiler/cross/bin/$(TARGET)-as
LD_EXEC ?= toolchain/cross_compiler/cross/bin/$(TARGET)-ld

# Find all the .cpp and .s files in the kernel directory and replace them with .o
KERNEL_SOURCES := $(shell find kernel/src -name '*.cpp' -o -name '*.s')
kernel = $(KERNEL_SOURCES:kernel/src/%=%)
kernel := $(kernel:.cpp=.o)
kernel := $(kernel:.s=.o)
kernel := $(addprefix obj/kernel/, $(kernel))

### Kernel ###

obj/kernel/%.o: kernel/src/%.cpp
	mkdir -p $(@D)
	$(GCC_EXEC) $(GCC_PARAMS) -Ikernel/include -c -o $@ $<

obj/kernel/%.o: kernel/src/%.s
	mkdir -p $(@D)
	$(AS_EXEC) $(AS_PARAMS) -Ikernel/include -o $@ $<

### Build ###

maxOS.bin: linker.ld $(kernel)
	$(LD_EXEC) $(LD_PARAMS) -T $< -o $@ $(kernel)
	objcopy --only-keep-debug $@ maxOS.sym

.PHONY: filesystem
filesystem:
	cd toolchain && ./copy_filesystem.sh
	sync

updateVersion:
	cd toolchain && ./version.sh

build:
	# Check if the cross compiler is made
	(ls toolchain/cross_compiler/cross/bin/$(TARGET)-gcc && echo yes) || make cross_compiler

	# Set the version
	make updateVersion

	# Make the kernel
	make maxOS.bin

	# Make the disk image
	ls maxOS.img || make disk_img
	make filesystem

all: $(kernel)

iso: maxOS.bin
	mkdir -p iso/boot/grub
	cp maxOS.bin iso/boot/maxOS.bin
	cp filesystem/boot/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o maxOS.iso iso
	rm -rf iso

### Run ###

qemu: build
	cd toolchain && ./run_qemu.sh --port-forwarding-host 1234 --port-forwarding-guest 1234

qemuDebug: build
	cd toolchain && ./run_qemu.sh --debug

qemuGDB: build
	make qemuDebug
	gdb -ex 'set remotetimeout 300' -ex 'target remote localhost:1234' -ex 'symbol-file maxOS.sym'

virtualbox: iso
	# Run the virtual machine
	"/mnt/c/Program Files/Oracle/VirtualBox/VirtualBoxVM.exe" --startvm "Max OS"

### Other Tools ###

.PHONY: clean
clean:
	rm -rf obj
	rm -f maxOS.bin maxOS.sym maxOS.img

cross_compiler:
	cd toolchain && ./make_cross_compiler.sh

disk_img:
	cd toolchain && ./create_disk_img.sh

install_deps:
	sudo apt-get update -y
	sudo apt-get install -y grub-pc qemu-system-i386 gdb dosfstools bridge-utils xorriso


