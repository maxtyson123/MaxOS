GCCPARAMS = -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = obj/loader.o \
 		  obj/gdt.o \
 		  obj/drivers/driver.o \
 		  obj/hardwarecommunication/port.o \
 		  obj/hardwarecommunication/interruptstubs.o \
 		  obj/hardwarecommunication/interrupts.o \
 		  obj/hardwarecommunication/pci.o \
	   	  obj/multitasking.o \
 		  obj/drivers/keyboard.o \
 		  obj/drivers/mouse.o \
 		  obj/drivers/vga.o \
 		  obj/gui/widget.o \
 		  obj/gui/window.o \
 		  obj/gui/desktop.o \
 		  obj/gui/render.o \
 		  obj/gui/widgets/text.o \
 		  obj/kernel.o

.PHONY: default
default: build;

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

maxOS.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

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