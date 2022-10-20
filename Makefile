GCCPARAMS = -m32 -Ikernel/include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

kernel =  obj/kernel/loader.o \
 		  obj/kernel/gdt.o \
 		  obj/kernel/drivers/driver.o \
 		  obj/kernel/hardwarecommunication/port.o \
 		  obj/kernel/hardwarecommunication/interruptstubs.o \
 		  obj/kernel/hardwarecommunication/interrupts.o \
 		  obj/kernel/hardwarecommunication/pci.o \
	   	  obj/kernel/multitasking.o \
 		  obj/kernel/drivers/keyboard.o \
 		  obj/kernel/drivers/mouse.o \
 		  obj/kernel/drivers/vga.o \
 		  obj/kernel/gui/widget.o \
 		  obj/kernel/gui/window.o \
 		  obj/kernel/gui/desktop.o \
 		  obj/kernel/gui/render.o \
 		  obj/kernel/gui/widgets/text.o \
 		  obj/kernel/kernel.o

.PHONY: default
default: build;

#Kernel

obj/kernel/%.o: kernel/src/%.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

obj/kernel/%.o: kernel/src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

buildKernel: $(kernel)
	echo Kernel Built

maxOS.bin: linker.ld $(kernel)
	ld $(LDPARAMS) -T $< -o $@ $(kernel)

maxOS.iso: maxOS.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp $< iso/boot
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/maxOS.bin'    	 >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=maxOS.iso iso
	rm -rf iso

build: maxOS.iso
	echo Complete


runQ: maxOS.iso
	qemu-system-i386 -boot d -cdrom maxOS.iso -m 512

runQ_W: maxOS.iso
	"C:\Program Files\qemu\qemu-system-i386" -boot d -cdrom maxOS.iso -m 512



install_dep:
	sudo apt-get update -y
	sudo apt-get install g++ binutils libc6-i386 grub-pc xorriso mtools

.PHONY: clean
clean:
	rm -rf obj