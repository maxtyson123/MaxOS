
GCC_PARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
GCC_EXEC ?= gcc

AS_PARAMS = --32
LD_PARAMS = -melf_i386
QEMU_PARAMS = -net user -net nic,model=pcnet,macaddr=08:00:27:EC:D0:29 -boot d -cdrom maxOS.iso -m 512 -hda maxOS.img -serial mon:stdio

kernel =  obj/kernel/loader.o \
 		  obj/kernel/system/gdt.o \
 		  obj/kernel/system/memorymanagement.o \
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
 		  obj/kernel/drivers/keyboard.o \
 		  obj/kernel/drivers/mouse.o \
 		  obj/kernel/drivers/vga.o \
 		  obj/kernel/drivers/ata.o \
 		  obj/kernel/drivers/amd_am79c973.o \
 		  obj/kernel/gui/widget.o \
 		  obj/kernel/gui/window.o \
 		  obj/kernel/gui/desktop.o \
 		  obj/kernel/gui/render.o \
 		  obj/kernel/gui/widgets/text.o \
 		  obj/kernel/common/printf.o \
 		  obj/kernel/net/etherframe.o \
 		  obj/kernel/net/arp.o \
 		  obj/kernel/net/ipv4.o \
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

### Make ###

maxOS.bin: linker.ld $(kernel) $(libraries) $(ports) $(programs)
	ld $(LD_PARAMS) -T $< -o $@ $(kernel) $(libraries) $(ports) $(programs)

maxOS.iso: maxOS.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp $< iso/boot
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "Max OS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/maxOS.bin'    	 >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=maxOS.iso iso
	rm -rf iso

build: maxOS.iso
	echo Complete

## MISC

setupQ:
	sudo apt-get install qemu-system-i386
	qemu-img create maxOS.img 10000


runQ: maxOS.iso
	qemu-system-i386 $(QEMU_PARAMS)

runQ_W: maxOS.iso
	"C:\Program Files\qemu\qemu-system-i386" $(QEMU_PARAMS)



install_dep:
	sudo apt-get update -y
	sudo apt-get install g++ binutils libc6-i386 grub-pc xorriso mtools

.PHONY: clean
clean:
	rm -rf obj