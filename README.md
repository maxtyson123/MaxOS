
# Max Os

Max os is based on: [OS Dev Wiki](https://wiki.osdev.org/Creating_an_Operating_System), [YouTube WYOOS](https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&ab_channel=WriteyourownOperatingSystem'), [KayOS]('#')

This is a hobby OS developed in mainly C++, the aim of this project is to setup a functional operating system that supports programs, has a GUI and is POSIX compatible. 

In the future I aim to have it run on a raspberry PI, include its very own in house web browser and a custom game engine.

The codebase is well commented with additional notes in the docs directory. Contribution is welcome, however it would be ideal once the basics are set up and I begin on the extended features. (See projects)

(BUGGY) Now with custom build toolchain (binutils, gcc, g++, make, etc) and a custom that can optionally be installed via the make_toolchain.sh file located in toolcahin. The OS can be built using this (build_via_tc.sh) or built via the make file (see below.) The toolchain will become more mainstream with the release of the c libraries.

MaxOS now has support for hardrives (Fat32 filesytem) and can be booted from an ATA drive. The makefile will mount MaxOS onto mnt/maxOS_img_1, however it is not ideal to directly copy files onto the mount point as upon build the folders are wiped and instead you should interact with the folders in the "filesystem" directory. (Note: when you reboot your device you need to run toolchain/create_disk_img.sh to remount the disk)

[![wakatime](https://wakatime.com/badge/github/maxtyson123/max-os.svg)](https://wakatime.com/badge/github/maxtyson123/max-os)
![maxOS](https://github.com/maxtyson123/max-os/workflows/maxOS/badge.svg)

 
## Screenshots
Booting
![Screenshot](docs/Screenshots/Boot/Console%20v2.png) cd 

GUI Window System
![Screenshot](docs/Screenshots/GUI/Windows%20(VESA).png)



### Whats working:
- [x] Bootloader
- [x] Global Descriptor Table
- [x] Interrupt Descriptor Table
- [x] Keyboard and Mouse Drivers
- [x] PCI Communication (for drivers)
- [x] GUI Framework (Front to back drawing, Area invalidation, etc)
- [x] Process Switching / Multitasking
- [x] Memory Management
- [x] Ethernet Networking Drivers
- [x] Various Internet Protocols (ARP, ICMP, UDP, TCP)
- [ ] {FIX Later} Fat32 Filesystem through an ATA driver


###  Future Plans

Kernel Cleanup

- [x] Timer rewrite
- [x] GUI Draw Rewrite
- [x] Console rewrite
- [x] VESA Video Mode                              
- [x] Kernel Boot Rewrite
- [x] Rewrite Event Handlers
- [x] Network Touch Up
- [ ] DCHP protocol
- [ ] GUI Bug Fixes
- [ ] Better Commenting and Doxygen Documentation
- [ ] Use better c++ coding conventions
- [ ] Fix filesystem (move to linux one?)
- [ ] HTTP Protocol
- [ ] USB
- [ ] Custom Toolchain
- [ ] CMAKE Build System (and maybe get it building and debugging in CLion)
- [ ] Clean up all the TODOs

Road to Userspace pt 1 - kernel core features

- [ ] New Process Manager / Scheduler
- [ ] Interprocess Communication
- [ ] Might need an I/O Rewrite (more to come)
- [ ] Better Memory Management
- [ ] Paging - ?
- [ ] Filesystem Fixes
- [ ] System Calls

Road to Userspace pt 2 - kernel supporting features

- [ ] Elf Loader
- [ ] Shell
- [ ] OS Specific Toolchain
- [ ] LibC
- [ ] LibM
- [ ] Services, (GUI Server, Network Server, etc)
- [ ] LibNet - SMB Needed


OS Functionality
- [ ] ext2 Filesystem
- [ ] GUI Theming, More GUI Widgets
- [ ] Game Ports (DOOM etc..)
- [ ] Users & Privileges
- [ ] Virtual Memory
- [ ] More drivers, essential ones etc, wifi maybe
- [ ] Microkernel
- [ ] 64 Bit
- [ ] 3D Rendering e.g. OpenGL
- [ ] Python Port
- [ ] Game Engine
- [ ] Web Browser
- [ ] M++
- [ ] VNC

### Services
Current:
- None

Planned:
- GUI Server
- Network Server

### GUI Programs
Current:
- Debug Console

Planned:
- Telnet Server
### CLI Programs
Current:
- None

Planned:
- UNIX Shell
### Libraries
Current:
- None

Planned:
- LibC
- LibM
- LibNet
- LibGUI
### Ports
Current:
- None

Planned:
- Git
- DOOM
- Bash
## Run Locally 

Steps for linux:

Clone the project

```bash
  git clone https://github.com/maxtyson123/max-os
```

Go to the project directory, fix the scripts (GitHub does weird line endings)

```bash
  cd max-os
  toolchain/fix_scripts_github.sh
```

Install Dependencies & build

```bash
 make install_dep 
 make build
```

Run Os In Qemu

```bash
make runQ
```