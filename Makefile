
TARGET=i686-elf

GCC_EXEC ?= $$HOME/opt/cross/bin/$(TARGET)-gcc
GCC_PARAMS = -ffreestanding -fno-exceptions -fno-rtti -nostdlib

AS_EXEC ?= $$HOME/opt/cross/bin/$(TARGET)-as

QEMU_PARAMS = -device pcnet,netdev=net0 \
              -netdev user,id=net0,hostfwd=tcp::1234-:1234 \
		      -m 512 \
		      -hda maxOS.img \
		      -vga std \
		      -serial stdio
QEMU_EXTRA_PARAMS? = ""

#For intel_i217 ethernet: 		-nic tap,model=e1000 \
#For amd_am79c973 ethernet: 	-net user -net nic,model=pcnet \
#Boot iso:       				-boot d -cdrom maxOS.iso \
#Boot from hdd:  				-boot c -hda /dev/loop0 \
#VESA graphics: 				-vga std \

kernel =  obj/kernel/loader.o \
 		  obj/kernel/system/gdt.o \
 		  obj/kernel/memory/memorymanagement.o \
 		  obj/kernel/memory/memoryIO.o \
 		  obj/kernel/drivers/driver.o \
 		  obj/kernel/hardwarecommunication/port.o \
 		  obj/kernel/hardwarecommunication/interruptstubs.o \
 		  obj/kernel/hardwarecommunication/interrupts.o \
 		  obj/kernel/hardwarecommunication/serial.o \
 		  obj/kernel/system/syscalls.o \
 		  obj/kernel/system/multithreading.o \
 		  obj/kernel/system/process.o \
 		  obj/kernel/hardwarecommunication/pci.o \
 		  obj/kernel/system/multitasking.o \
 		  obj/kernel/drivers/peripherals/keyboard.o \
 		  obj/kernel/drivers/peripherals/mouse.o \
 		  obj/kernel/drivers/video/video.o \
 		  obj/kernel/drivers/video/vga.o \
 		  obj/kernel/drivers/video/vesa.o \
 		  obj/kernel/drivers/ata.o \
 		  obj/kernel/drivers/ethernet/amd_am79c973.o \
 		  obj/kernel/drivers/ethernet/intel_i217.o \
 		  obj/kernel/drivers/ethernet/ethernet.o \
 		  obj/kernel/drivers/clock/clock.o \
 		  obj/kernel/drivers/console/console.o \
 		  obj/kernel/drivers/console/textmode.o \
 		  obj/kernel/drivers/console/vesaboot.o \
 		  obj/kernel/filesystem/filesystem.o \
 		  obj/kernel/filesystem/fat32.o \
 		  obj/kernel/filesystem/msdospart.o \
 		  obj/kernel/gui/widget.o \
 		  obj/kernel/gui/window.o \
 		  obj/kernel/gui/desktop.o \
 		  obj/kernel/gui/font.o \
 		  obj/kernel/gui/widgets/text.o \
 		  obj/kernel/gui/widgets/button.o \
 		  obj/kernel/gui/widgets/inputbox.o \
 		  obj/kernel/common/graphicsContext.o \
 		  obj/kernel/common/colour.o \
 		  obj/kernel/common/inputStream.o \
 		  obj/kernel/common/outputStream.o \
 		  obj/kernel/net/ethernetframe.o \
 		  obj/kernel/net/arp.o \
 		  obj/kernel/net/ipv4.o \
 		  obj/kernel/net/icmp.o \
 		  obj/kernel/net/udp.o \
 		  obj/kernel/net/tcp.o \
 		  obj/kernel/kernel.o

.PHONY: default
default: build;

### Kernel ###

obj/kernel/%.o: kernel/src/%.cpp
	mkdir -p $(@D)
	$(GCC_EXEC) $< -o $@ $(GCC_PARAMS) -Ikernel/include

obj/kernel/%.o: kernel/src/%.s
	mkdir -p $(@D)
	$(AS_EXEC) -Ikernel/include -o $@ $<

### Build ###

maxOS.bin: linker.ld $(kernel)
	ld -melf_i386 --verbose -T $< -o $@ $(kernel)
	objcopy --only-keep-debug $@ maxOS.sym

.PHONY: filesystem
filesystem:
	toolchain/copy_filesystem.sh
	sync

incrementVersion:
	toolchain/increment_version.sh

build: maxOS.bin
	@echo Made Max OS Kernel

	# Make the disk image
	(ls maxOS.img && echo yes) || toolchain/create_disk_img.sh
	make filesystem

	@echo === Made Max OS ===
	make incrementVersion

qemu: build
	qemu-system-i386 $(QEMU_PARAMS) $(QEMU_EXTRA_PARAMS)

debug_qemu: build
	x-terminal-emulator -e make runQ QEMU_EXTRA_PARAMS="-s -S" & gdb -ex 'set remotetimeout 300' -ex 'target remote localhost:1234' -ex 'symbol-file maxOS.sym'

.PHONY: clean
clean:
	rm -rf obj

cross_compiler:
	cd toolchain && ./make_cross_compiler.sh

test:
	echo $(kernel)