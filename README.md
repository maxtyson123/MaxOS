
# Max Os

Max os following: [OS Dev Wiki](https://wiki.osdev.org/Creating_an_Operating_System), [YouTube WYOOS](https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&ab_channel=WriteyourownOperatingSystem')

Made mainly in C++ with a dabble of ASM. [![wakatime](https://wakatime.com/badge/github/maxtyson123/max-os.svg)](https://wakatime.com/badge/github/maxtyson123/max-os)

Currently working on: Kernel - 

##Screenshots
![Screenshot](docs/Screenshots/Drivers, PCI.png)

### Services
-None Yet
### GUI Programs
-None Yet
### CLI Programs
-(COMING) Uinx / POSIX like 
### Libraries
-(COMING) Libc
-(COMING) Libm
### Ports
-(COMING) Doom

## Progress

###  Initial 

- [x] Memory and Global Descriptor Table
- [x] Hardware Communication / Ports
- [x] System Interrupts
- [x] Keyboard & Mouse
- [x] Drivers
- [x] PCI
- [ ] GUI Basics
- [ ] Desktop and Windows
- [ ] Network
- [ ] Filesystem 
- [ ] Audio


###  Extended

- [ ] Usable Desktop
- [ ] Libs
- [ ] Users 
- [ ] Shell
- [ ] 64 Bit
- [ ] Applications
- [ ] Game Engine
- [ ] Web Browser
- [ ] Game Ports (DOOM etc..)
- [ ] M++
- [ ] POSIX


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
 sudo apt-get install g++ binutils libc6-dev-i386 grub-legacy xorriso
 make maxos.iso  
```

Run Os 

```bash
     Virtual Box
```


