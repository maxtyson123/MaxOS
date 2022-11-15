
# Max Os

Max os following: [OS Dev Wiki](https://wiki.osdev.org/Creating_an_Operating_System), [YouTube WYOOS](https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&ab_channel=WriteyourownOperatingSystem')

This is a hobby OS developed in mainly C++, the aim of this project is to setup a functional operating system that supports programs, has a GUI and is POSIX compatible. 

In the future I aim to have it run on a raspberry PI, include its very own in house web browser and a custom game engine.

The codebase is well commented with additional notes in the docs directory. Contribution is welcome, however it would be ideal once the basics are set up and I begin on the extended features. (See projects)

Now with custom build toolchain (binutils, gcc, g++, make, etc) and a custom that can optionally be installed via the make_toolchain.sh file located in toolcahin. The OS can be built using this (build_via_tc.sh) or built via the make file (see below.) The toolchain will become more mainstream with the release of the c libraries.

[![wakatime](https://wakatime.com/badge/github/maxtyson123/max-os.svg)](https://wakatime.com/badge/github/maxtyson123/max-os)
![maxOS](https://github.com/maxtyson123/max-os/workflows/maxOS/badge.svg)

 
## Screenshots
![Screenshot](docs/Screenshots/ATA_Hardrives.png)

### Services
-None Yet
### GUI Programs
-Debug Console
### CLI Programs
-(COMING) Uinx / POSIX like 
### Libraries
-(COMING) Libc
-(COMING) Libm
### Ports
-(COMING) Doom

## Progress

###  Intial

Moved to  projects page


###  Extended
- [ ] VESA Video Mode
- [ ] Usable Desktop
- [ ] New Process Manager / Scheduler
- [ ] Elf Loader
- [ ] Shell
- [ ] System Calls
- [ ] Libc
- [ ] GCC Cross compiler
- [ ] POSIX
- [ ] Game Ports (DOOM etc..)
- [ ] Users & Privileges
- [ ] Virtual Memory
- [ ] 64 Bit
- [ ] Game Engine
- [ ] Web Browser
- [ ] M++
- [ ] VNC


## Run Locally

Clone the project

```bash
  git clone https://github.com/maxtyson123/max-os
```

Go to the project directory

```bash
  cd max-os
```

Make Files and OS

```bash
 make install_dep
 make maxOS.iso  
```

Run Os (Linux)

```bash
make setupQ
make runQ
```

Run Os (Windows)

```bash
# Install QEMU into "C:\Program Files\qemu\"
make runQ_W
```