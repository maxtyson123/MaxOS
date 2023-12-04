
GCC_PARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -g
TARGET=i686-elf
GCC_EXEC ?= $$HOME/opt/cross/bin/$(TARGET)-gcc

BUILD_COMPLETE ?= make runQ

AS_PARAMS = --32
LD_PARAMS = -melf_i386 --verbose
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

libraries =
ports =
programs =


.PHONY: default
default: build;

### Kernel ###

obj/kernel/%.o: kernel/src/%.cpp
	mkdir -p $(@D)
	$(GCC_EXEC) $(GCC_PARAMS) -Ikernel/include -c -o $@ $<

obj/kernel/%.o: kernel/src/%.s
	mkdir -p $(@D)
	as $(AS_PARAMS) -Ikernel/include -o $@ $<

### Libraries ###

obj/libraries/%.o: libraries/src/%.cpp
	mkdir -p $(@D)
	$(GCC_EXEC) $(GCC_PARAMS) -Ilibraries/include -c -o $@ $<

obj/libraries/%.o: libraries/src/%.s
	mkdir -p $(@D)
	as $(AS_PARAMS) -Ilibraries/include -o $@ $<

buildLibraries: $(libraries)
	echo Libraries Built


### Ports ###

obj/ports/%.o: ports/src/%.cpp
	mkdir -p $(@D)
	$(GCC_EXEC) $(GCC_PARAMS) -Iports/include -c -o $@ $<

obj/ports/%.o: ports/src/%.s
	mkdir -p $(@D)
	as $(AS_PARAMS) -Iports/include -o $@ $<

buildPorts: $(ports)
	echo Ports Built


### Programs ###

obj/programs/%.o: programs/src/%.cpp
	mkdir -p $(@D)
	$(GCC_EXEC) $(GCC_PARAMS) -Iprograms/include -c -o $@ $<

obj/programs/%.o: programs/src/%.s
	mkdir -p $(@D)
	as $(AS_PARAMS) -Iprograms/include -o $@ $<

buildPrograms: $(programs)
	echo Programs Built

### Build ###

maxOS.bin: linker.ld $(kernel) $(libraries) $(ports) $(programs)
	ld $(LD_PARAMS) -T $< -o $@ $(kernel) $(libraries) $(ports) $(programs)
	objcopy --only-keep-debug $@ maxOS.sym

.PHONY: filesystem
filesystem:
	toolchain/copy_filesystem.sh
	sync

incrementVersion:
	toolchain/increment_version.sh

build: maxOS.bin
	@echo Made Max OS Kernel

	# Make the libraries
	@echo Made Max OS Libraries

	# Make the programs
	@echo Made Max OS Programs

	# Make the ports
	@echo Made Max OS Ports

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