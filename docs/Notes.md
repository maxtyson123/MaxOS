# Notes
These are my notes relative to various parts of the OS

# Contents
<!-- TOC start (generated with https://github.com/derlin/bitdowntoc) -->

- [Hardware Communication](#hardware-communication)
  + [Data Send and Receive](#data-send-and-receive)
  + [Interrupts](#interrupts)
  + [Peripheral Component Interconnect (PCI)](#peripheral-component-interconnect-pci)
  + [base Address Registers](#base-address-registers)
  + [RTC Timer](#rtc-timer)
- [Files And Drives](#files-and-drives)
  + [Hard-drives](#hard-drives)
  + [Partion Table](#partion-table)
  + [Fat 32 File System](#fat-32-file-system)
- [Graphics](#graphics)
  + [Graphics Mode {VGA}](#graphics-mode-vga)
  + [GUI - Framework](#gui-framework)
- [System Stuff](#system-stuff)
  + [Multitasking](#multitasking)
  + [m_threads](#threads)
  + [Dynamic Memory Management / Heap](#dynamic-memory-management-heap)
  + [System Calls](#system-calls)
- [Networking](#networking)
  + [Driver am79c971](#driver-am79c971)
  + [Handler](#handler)
  + [Address Resolution Protocol](#address-resolution-protocol)
  + [Internet Protocol v4](#internet-protocol-v4)
  + [Internet Control Message Protocol](#internet-control-message-protocol)
  + [User Datagram Protocol](#user-datagram-protocol)
  + [Transmission Control Protocol](#transmission-control-protocol)
  + [Hyper Text Transfer Protocol](#hyper-text-transfer-protocol)
  + [Domain Name System](#domain-name-system)
  + [Dynamic Host Configuration Protocol](#dynamic-host-configuration-protocol)
  + [Secure Shell](#secure-shell)
  + [File Transfer Protocol](#file-transfer-protocol)
  + [Simple Mail Transfer Protocol](#simple-mail-transfer-protocol)
  + [Open Systems Interconnection Model](#open-systems-interconnection-model)
- [Index](#index)
  + [Kernel](#kernel)

<!-- TOC end -->

NOTE TO SELF: FUNC AS VARIABLE:  auto func_name = [&](uint32_t p) {
  // Code
};

<!-- TOC --><a name="hardware-communication"></a>
# Hardware Communication
Here are some notes on how the communication with hardware works, this is used for the keyboard and mouse communication and setting up other devices, e.g. GPU
<!-- TOC --><a name="data-send-and-receive"></a>
### Data Send and Receive
This relates to "port.cpp", directly used by "interrupts.cpp, mouse.cpp, keyboard.cpp"
- When you press a key on the keyboard a signal will go to the programmable interrupt controller (PIC)
- By default, the PIC is set ignore that, so to receive the key signal tell PIC not to ignore it
- This is done by sending some data to the PIC
- This means there has the to be a function of talking to the hardware (send data out / receive data in)
- CPU has a multiplexer and a de-multiplexer which is connected to different hardware which is what physically sends the data
- The PIC (a piece of hardware) has the port number 32 (0x20), so the data for the PIC has to be sent port 32 via the multiplexer
- Using assembler, data can be set:
```bash
 outb(portNumber, data)
 outb(0x20, 0x1) //Example using PIC (port 32) and the data 1
```
- To initialize the PIC  ICWs (Initialization Control Words) must be sent
<!-- TOC --><a name="interrupts"></a>
### Interrupts
This relates to "interrupts.cpp, interruptstubs.s", which are extended by "keyboard.cpp, mouse.cpp".
See also [Access Rights](https://wiki.osdev.org/Security#Rings), [IDT](https://wiki.osdev.org/Interrupt_Descriptor_Table), [Stack](https://www.geeksforgeeks.org/introduction-of-stack-based-cpu-organization/)
- If the PIC sends information without the Interrupt Descriptor Table (IDT) then there will just be a "protection fault"
- This is because the PIC relays information in the form of interrupts which the OS doesn't know how to handle without an IDT most likely causing it ro crash or restart
- The information that goes into this table (IDT) is:
  - Interrupt number: uint8_t (unsigned 8bit integer)
  - handler: address in the ram of where to jump to preform the code/function to handle this interrupt
  - Flags:
  - Segment: tells the processor to switch a certain segment before executing handler, (this is for when the processor is executing code in the userspace, the handler is in the kernelspace and the processor needs to switch to the kernel segment)
  - Access Rights: Number from 0-3 (3 is userspace)
- To do this in C++ it would be ideal to receive the Interrupt number as a parameter and handel it in a function eg:
```bash
 void interruptHandler(uint8_t interruptNum, ...){
   //EXAMPLES: Handle interupt x 
   if(interruptNum == 1){
      printf("Keyboard key A ");
     }
   if(interruptNum == 2){
        printf("Mouse moved ");
     }
 }
```
- To get this number as a parameter the CPU would have to push the number onto a stack, which the CPU cant do safely
- So instead there has to be function for every interrupt, Which can be done most safely in assembly instead of c++ because compilers be compiling
- From the assembly code a c++ function can then be called (similar to calling kernelMain in loader.s)
- Once the Interrupt has been received and handled the OS then needs to send a reception message for it continue.
- Answers (receive messages) only need to be sent to hardware interrupts

<!-- TOC --><a name="peripheral-component-interconnect-pci"></a>
### Peripheral Component Interconnect (PCI)
This relates to "pci.cpp", See also [PCI](https://www.lowlevel.eu/wiki/Peripheral_Component_Interconnect)
- In the computer there is a PCI controller that is connected to the PIC. Devices connected to the PCI controller are generally a GPU (graphics card) or a network/Wi-Fi card
- The PCI controller has up to 8 BUS-es, and on each BUS there can be up to 32 different devices
- Devices can have up to 8 functions, for example the sound card has audio capture and audio output
- 3 bits to encode BUS, 5 bits to encode the device number,3 bits to encode the function
- Unlike the keyboard and mouse, the port number cant be hardcoded because it is unknown if the device is even there or put in the slot on the motherboard
- As a workaround, the PCI controller has a function to query what device is in the specified port
- Iterating through the BUS-es and querying the device number will return an uint16_t for the vendor ID and an uint16_t for device ID.
- The PCI controller will also tell more general information such as class and subclass IDs, which are useful for compatibility modes
- Once the PCI driver is set up then it becomes easy to extend the OS as in future all that needs to be done is to get the device ID and write drivers for them
<!-- TOC --><a name="base-address-registers"></a>
### base Address Registers
- The solution for having multiple PCI devices (two GPUS, two screens etc…) is to have a base Address Registers (BAR)
- base address registers are registers in the PCI configuration space, which in a more simple way is just storing the address of a PCI device in memory
- There are two types of BARs, one is  the I/O BAR, this is for devices that communicate bit by bit e.g. mouse where the m_position is read from the port each interrupt
- The I/O base address register is 4 bytes long:
- - Lowest bit:            Type Of register: (I/O bit = 1)
- - Second-lowest bit:     Reserved
- - Bits 3 - 16:           Port Number (MUST BE MULTIPLE OF 4)
- Memory mapping is another type of BAR, this one is where the device takes x memory location, and to communicate with it just write/read from that location in memory. This is better performance-wise as the CPU can do other m_tasks while that is updating in the m_background
- The Memory Mapping BAR is 4 bytes long:
- - Lowest Bit:  Type Of register: (MemMap bit = 0)
- - Second and third: (00 = 32bit BAR) or (01 = 20bit BAR) or (10 = 64bit BAR)
- - Fourth bit: Prefetch-able bit (e.g. reading from a hard drive in advance because its estimated that the program will need that soon)
- - Last Twelve: Ram Address (MUST BE MULTIPLE OF 4)
<!-- TOC --><a name="rtc-timer"></a>
### RTC Timer
See also [RTC](https://wiki.osdev.org/RTC)
- The RTC (Real Time Clock) is a device that is connected to the PCI controller and is used to keep track of time
- The ports used for the RTC are 0x70(write) and 0x71(read)
- The base frequency of the RTC is 32768 Hz, it is possible to change this frequency but that can mess with the time keeping
- The RTC has a register called the CMOS RAM, which is a 128 byte long memory that can be read and written to, it stores the time and date
- To get the time certain registers need to be read from te data port (0x71). Before doing this updates need to be disabled so that the time is not changed while reading it.
- The registers are:
- - Seconds:    0x00 (0-59)
- - Minutes:    0x02 (0-59)
- - Hours:      0x04 (0-23)
- - WeekDay:    0x06 (1-7, 1 = Sunday)
- - Day:        0x07 (1-31)
- - Month:      0x08 (1-12)
- - Year:       0x09 (0-99)

<!-- TOC --><a name="files-and-drives"></a>
# Files And Drives
<!-- TOC --><a name="hard-drives"></a>
### Hard-drives
- History of hard drives: 20 years ago there used to be IDE (Integrated Device Electronics) that was later continued on to ATA (Advanced Technology Attachment) that has now been moved onto what is used to day SATA (Serial Advanced Technology Attachment)
- As hard drives have been around for a long time, they are well documented and easy to implement
- SATA devices are almost always compatible to ATA or AHCI (Advanced Host Controller Interface)
- Using the PCI controller, SATA hard drives can be found on class id 0x01 and subclass id 0x06
- There are two different ways of accessing a hard-drive. The m_first_memory_chunk one is PIO (Programed Input Output) which is relatively simple to implement but rather slow (maxs out at 16 M/Bs)
- The second and better way would be DMA (Direct Memory Access) , which simply just writes the data to a memory location and sends an interrupt once finished
- Although it is better to read from the PCI devices list, the ports can just be hard coded
- For the Implementation of PIO:
- - PIO Has 28Bit Mode and 48Bit Mode
- - These modes just specify how many bits are transferred to the device to request a sector (Sectors are 512 bytes)
- - 28Bits supports hard drives/files upto 4GB whereas 48Bits support hard drives/files upto 8PB
- Previously to communicate with data on the hard drive the OS had to know the CHS (Cylinder, Head, Sector)  address. As hard drive developed and there were no longer spinning disks and such (e.g. SSDs) a new way of comincating was invented: LBA.
- LBA stands for Logical Block Address which basically just returns a sector with a given number
<!-- TOC --><a name="partion-table"></a>
### Partion Table
- This OS will use the MS DOS partition table is because it is the most common and well documented. Additionalym Windows uses it and so does Linux
- This partion table is simple so it is easy to implement, but it is also limited to 4 primary partitions and 1 extended partition.
- The partition table is located at the end of the m_first_memory_chunk sector of the hard drive
- At the start of the m_first_memory_chunk sector there is a 440 byte boot sector, which is used to boot the OS. The bootstraper will then look at the partition table to find the OS partition.
- After the boot sector there are 4 bytes of signature.
- Then there is 2 bytes that are unused
- Then there is the partition table which is 64 bytes long and has 4 entries of 16 bytes each
- At the end there are 2 magic bytes (0x55, 0xAA) which is used to check if the partition table is m_valid
- The structure of the partition table is as follows:
- - First 1 byte: Bootable flag (0x80 = bootable, 0x00 = not bootable) (There can only be one bootable partition per hard drive)
- - Next 3 bytes: Cylinder Head Sector (CHS) address of the m_first_memory_chunk sector of the partition
- - Next 1 byte: Partition type (0x00 = empty, 0x01 = FAT12, 0x04 = FAT16, 0x05 = Extended, 0x06 = FAT16, 0x07 = NTFS, 0x0B = FAT32, 0x0C = FAT32, 0x0E = FAT16, 0x0F = Extended, 0x11 = Hidden FAT12, 0x14 = Hidden FAT16, 0x16 = Hidden FAT16, 0x1B = Hidden FAT32, 0x1C = Hidden FAT32, 0x1E = Hidden FAT16, 0x42 = MBR, 0x82 = Linux Swap, 0x83 = Linux, 0x84 = Hibernation, 0x85 = Linux Extended, 0x86 = NTFS Volume Set, 0x87 = NTFS Volume Set, 0xA5 = FreeBSD, 0xA6 = OpenBSD, 0xA9 = NetBSD, 0xB7 = BSDI, 0xB8 = BSDI Swap, 0xEB = BeOS, 0xEE = GPT, 0xEF = EFI System, 0xFB = VMWare File System, 0xFC = VMWare Swap)
- - Next 3 bytes: Cylinder Head Sector (CHS) address of the last sector of the partition
- - Next 4 bytes: LBA address of the m_first_memory_chunk sector of the partition
- - Next 4 bytes: Number of sectors in the partition
<!-- TOC --><a name="fat-32-file-system"></a>
### Fat 32 File System
See also [FAT32](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#FAT32)
- The FAT32 file system is the most common file system used in Windows and Linux
- The FAT32 file system is a simple file system that is easy to implement and is well documented
- The FAT32 file system is a cluster based file system, which means that the file system is divided into clusters. Each cluster is 4KB in size
- FAT32 was designed to be compatible with FAT16, which is why it has a 16bit root directory
- FAT32 has 3 sections: Reserved sector, the FAT32 table and the data section
- The structure of the FAT32 file system is as follows (512 bytes = 1 sector):
- - Reserved Sector: 32KB (64 sectors)
- - - Boot Sector: 512 bytes
- - - FS Information Sector: 512 bytes
- - - More optional reserved sectors
- - FAT32 Table: 4KB (8 sectors)
- - - FAT32 Table 1: 4KB (8 sectors)
- - - FAT32 Table 2: 4KB (8 sectors) (optional)
- - Data Section: To the end of the hard drive

- Inside the boot sector there is a BIOS Parameter Block (BPB)
- The structure of the BPB is as follows:
- - First 3 bytes: Jump instruction (Jump to the bootstraper)
- - Next 8 bytes: OEM Name (Name of the OS that created the file system)
- - Next 2 bytes: Bytes per sector (512)
- - Next 1 byte: Sectors per cluster (8)
- - Next 2 bytes: Reserved sectors (64)
- - Next 1 byte: Number of FATs (2)
- - Next 2 bytes: Number of root directory entries (512) (No longer used)
- - Next 2 bytes: Total number of sectors (0) (No longer used)
- - Next 1 byte: Media descriptor (Is this a hard drive or a floppy disk etc.)
- - Next 2 bytes: Sectors per FAT (0) (Used only in FAT16)
- - Next 2 bytes: Sectors per track (0) (No longer used)
- - Next 2 bytes: Number of heads (0) (No longer used)
- - Next 4 bytes: Number of hidden sectors (0) (No longer used)
- - Next 4 bytes: Total number of sectors (0) (No longer used)
- - Next 4 bytes: Table size (8)
- - Next 2 bytes: Extended flags (0)
- - Next 2 bytes: FAT version (0)
- - Next 4 bytes: Root directory cluster (2)
- - Next 2 bytes: FS Information sector (1)
- - Next 2 bytes: Backup boot sector (6)
- - Next 12 bytes: Reserved (0)
- - Next 1 byte: Drive number
- - Next 1 byte: Reserved (0)
- - Next 1 byte: Extended boot signature (0x29)
- - Next 4 bytes: Volume ID
- - Next 11 bytes: Volume label
- - Next 8 bytes: File system type (FAT32)
- In the filesystem a directory is a cluster (e.g. 4kb) and every sector in that cluster containers 16 directory entries
- The structure of a directory entry is as follows:
- - First 8 bytes: File name (8.3 format)
- - Next 3 bytes: File extension
- - Next 1 byte: File attributes (0x01 = read Only, 0x02 = Hidden, 0x04 = System, 0x08 = Volume ID, 0x10 = Directory, 0x20 = Archive, 0x40 = Device, 0x80 = Unused)
- - Next 1 byte: Reserved (0)
- - Next 7 bytes: Creation time
- - Next 2 bytes: Cluster high
- - Next 4 bytes: write time e.t.c
- - Next 2 bytes: Cluster low
- - Next 4 bytes: File size
- The cluster low and high are basically pointers to where the directory entry is located on the hard drive or where the file is located on the hard drive
- To read a file you need to read the cluster low and high and then read the cluster from the hard drive
- The FAT32 table is a table that contains pointers to where the next cluster is located on the hard drive
- When a hard drive is fragmented files larger than a cluster are split up and stored in multiple places on the hard drive
- The FAT32 table is used to find the next cluster of a file, when a file is fragmented
- To expand a file the File Allocation Table has to be edited to point to a new empty cluster, this is done by looping through all the clusters in the FAT and finding one with the value 0x000000 which means it is unused. Then the FAT is edited so that the end of the file is this new cluster, the cluster is marked as used and the finnaly the file is set to use this cluster.
- A cluster can only belong to one file at a time. This is useful as that means that when a file is being read or expand the operating system doesn't need to split the cluster up.
<!-- TOC --><a name="graphics"></a>
# Graphics
Here are the notes on graphics for the operating system. (May need to read hardware communication m_first_memory_chunk)
<!-- TOC --><a name="graphics-mode-vga"></a>
### Graphics Mode {VGA}
- To write pixels and such to the screen for a GUI the graphics card needs to be put into graphics mode (grub puts it into text mode by default)
- There are two ways of doing this, via the BIOS 0x13 Interrupt or by directly telling the GPU.
- As the BIOS interrupt 0x13 is only for 16Bit operating system (MaxOS is 32bit) graphics mode would have to be set via the GPU
- Setting graphics mode via the GPU is incredibly complicated therefore the script ['modes.c' by Chris Giese](https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c) is used
- To set the graphics mode via the GPU different sets of values have to be sent to different places, for example a 320px wide by 200px m_height x 8-bit colour depth graphics mode would have to be set by sending these values:
- - Misc Port: 0x63,
- - Sequencer Port:  0x03, 0x01, 0x0F, 0x00, 0x0E,
- - (CRTC) Cathode ray tube controller Port:  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF,
- - (GC) Graphics controller Port: 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
- - (AC) Attribute Controller: 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00
- However, communicating with these ports aren't that easy as there is an index port (to tell where the data should be put) and a data port (push the data)
- And on the CRTC port there is special indexes and data that has to be set for it to unlocked before writing actual data (the reason it has to be unlocked is that if incorrect data is the hardware could be damaged)
- Similarly, the AC port has to be reset before each index & data write.
- The VGA class that was written should have a class above that call m_graphics_context which would define methods such as render_pixel(...), drawRect(...), draw_line(...), draw_circle(...) and sub-classes such as color.
- The draw functions should rely on the render_pixel function therefore in different graphics modes that are derived from that m_graphics_context class can be supported.

### VESA
- VESA is a legacy extension to VGA that allows for a higher color depth and larger screen resolutions
- VESA is a standard that is supported by most graphics cards
- The VESA framebuffer can be acessed from the GRUB Multibooot structure
- The VESA framebuffer is a memory region that contains the pixels that are displayed on the screen
- The VESA framebuffer is a linear framebuffer, which means that the pixels are stored in a linear array
- VESA Supports these  modes:
- - MODE    RESOLUTION  COLOUR DEPTH    MAXIMUM COLORS
- - 0x100   640x400     8               256
- - 0x101   640x480     8               256
- - 0x102   800x600     4               16
- - 0x103   800x600     8               256
- - 0x10D   320x200     15              32768
- - 0x10E   320x200     16              65536
- - 0x10F   320x200     24/32           16777216
- - 0x110   640x480     15              32768
- - 0x111   640x480     16              65536
- - 0x112   640x480     24/32           16777216
- - 0x113   800x600     15              32768
- - 0x114   800x600     16              65536
- - 0x115   800x600     24/32           16777216
- - 0x116   1024x768    15              32768
- - 0x117   1024x768    16              65536
- - 0x118   1024x768    24/32           16777216
- Some BIOS support 24bit color depth whilst others support 32bit color depth
<!-- TOC --><a name="gui-framework"></a>
### GUI - Framework
See also: Bresenham line algorithm (use for later)
- The GUI framework will have a base class called Widget, which should have a draw function. This draw function would get an m_graphics_context and draw itself onto of it using the graphicContext's draw methods.
- Thees widgets will have parents and such, therefore their draw positions will be relative, meaning there has to be a function to get the absolute m_position.
- Widgets will also have an m_width and m_height, and a function to check if a co-ordinate is inside itself (used for mouse handling and such)
- To handle mouse input, the widget will use a mouse event handler and handle the events onMouseDown, on_mouse_move_widget, on_mouse_up_event. However, the mouse handler won't be on the widget itself, rather it would be on the desktop.
- This is because the mouse movement is reported in relative-ness not absolute m_position (desktop would store mouse m_position and update it based on the movement, the object can just query the x,y pos from the desktop).
- A subclass of the widget would be a composite class, which would contain an array of child widgets, and it would pass methods on to the m_children (e.g. the child gets drawn last so its on m_top)
- To get which widget to preform the keyEvent or mouseEvent the desktop will have a function of getting the focused widget
- The desktop will work like the composite widget, but will have to override the mouseHandler class as mouse passes movement in relative to last m_position (moved x  pixels) so the desktop will have to translate the restiveness into absolute positions
- For better performance, instead of re-drawing the screen every time just draw it once and then when a gui object changes (e.g. moving a window) memory the object to the new state and then redraw any invalid parts (invalid is where places where there used to be the gui object)
- To draw text the "modes.c" has various 8x8 Bitmap fonts. Using these fonts a Text widget can be made that gets the characters of a string from the m_font and then print all the pixels in that character.

<!-- TOC --><a name="system-stuff"></a>
# System Stuff
<!-- TOC --><a name="multitasking"></a>
### Multitasking
See also [Stack](https://wiki.osdev.org/Stack), **Note this needs better explaining
- In the RAM there is different places for different species of code. The processor jumps around executing different parts. Somewhere in the RAM there is a STACK where stuff is being pushed a popped
- Before Multitasking: A timer interrupt occurs and what happens is the processor pushes some registers on the stack (e.g. pointer to where it was executing before the interrupt). The processor then goes into the interrupt manager (see interrupts.cpp) and executes the handler for that interrupt. Once the handler has finished executing the processor then carry's on what it was executing before (hence why the pointer was pushed to the stack).
- However, to do multithreading the current way of executing in the OS needs to change. First the OS will take another portion of the RAM, reserve it for multitasking, and make its own stack there for every task.
- Therefore, when a task is created a memory region is made. Information for the processor is then explicitly writen into segments in that region e.g. the instruction pointer at the entry point
- Previously: the interstubs.s file pushed the current processor values (interstubs line 64: save the registers) and then pushed the stack pointer (interstubs line 80: Invoke C++ m_handlers) and then load register again
- (Carrying on "Previously") The handler function would have the pointer to the stack (passed as var: esp). The return value of handle interrupt is then written into esp and then executed by processor.
- Now with multitasking the return value for the handle interrupt function would be changed to point to the m_top of the new task's stack.
- To sum up:
- - The kernel will be executing function "X" and receive an interrupt
- - kernel will execute the interrupt handler
- - The interrupt handler will then return a pointer to the new task
- - Kernel will then execute the new task instead of executing function "X"
- To schedule processes fairly, a round-robin scheduler generally employs time-sharing, giving each job a time slot or quantum (its allowance of CPU time), and interrupting the job if it is not completed by then. The job is resumed next time a time slot is assigned to that task. If the task terminates or changes its state to waiting during its attributed time quantum, the scheduler selects the m_first_memory_chunk task in the ready queue to execute
<!-- TOC --><a name="threads"></a>
### m_threads
See also: [Thread](https://wiki.osdev.org/Thread)
- A thread is a lightweight process. It is a sequence of instructions within a program that can be executed independently of other code. m_threads can be created and managed by a thread library.
- m_threads are often used in parallel computing, where they can be used to perform multiple m_tasks simultaneously. m_threads are also used in multimedia, where they can be used to perform different m_tasks in real time. For example, a thread can be used to play a sound while another thread is used to display a graphic.
- m_threads are also used in operating systems, where they can be used to perform different m_tasks simultaneously. For example, a thread can be used to play a sound while another thread is used to display a graphic.
- The main difference between a thread and a process is that threads share the same address space, while processes have separate address spaces. This means that threads can share data and code, while processes cannot. This also means that threads are faster than processes, because they do not need to be loaded into memory.
- To implement threads in the OS, the kernel will have to implement a thread class. This class will have a function to create a thread, and a function to execute the thread. The thread class will also have a function to get the current thread.
- The thread class will also have a function to get the current thread. This function will be used to get the current thread, and then the thread will be able to access its own stack.
- Yeilding is when a thread gives up its time slice and allows another thread to run. This is done by calling the yield function on the thread class.
- Processes and threads are different. A process is a program in execution. A thread is a component of a process. A process can have multiple threads. m_threads are often used to implement concurrency within a process. m_threads are also used to implement parallelism, where multiple processes can run simultaneously on a multicore processor.
<!-- TOC --><a name="dynamic-memory-management-heap"></a>
### Dynamic Memory Management / Heap
See also [Double Linked List](https://en.wikipedia.org/wiki/Linked_list#Doubly_linked_list)
- Before Dynamic Memory Management (DMM), the implementation was very static as there would just be the objects and are allocated on the stack.
- This is not the best as sometimes you don't know what is needed in advance, e.g. when iterating through the PCI devices and there is a device that needs its driver instantiated there for space has to be allocated or reserved for that driver
- This means there needs to be a way of keeping track of the space that has already been assigned to something as it is not ideal to have stuff overlap and disturb each other.
- Currently, the RAM looks like this:
```bash
  [Video Memory] [Text Memory] [BIOS] [ Grub / Bootloader ] [ MaxOS / Kernel ] [ Stack ] [ FREE SPACE ]
```
- However, there is a problem as there is no way to know if GRUB has loaded something behind the kernel e.g. Multiboot structure or Modules . Therefore, it is unknown how much space there is
- To get around this the OS will just get the pointer to the stack and add 4MB to it. Which is not ideal as that is just a guess that is hopefully enough
- To implement this a class called memory manager will be given the stack point and a size of X. (Can get X from grub)
- The memory will then be split up into Memory Chunks (a class) that will be structured like the following:
- - Memory Chunk : Previous
- - Memory Chunk : Next
- - Bool : Allocated
- - size_t : size
- These Memory Chunks are just a linked list (double linked)  and every chunk that has dropped from the RAM will be entered into the list, which is how the OS will keep track of used / free memory
- More than one free chunk in a row is bad as it means larger data that could have had fit into those two chunks if they were combined won't be able to. This can be prevented merging a chunk with another if it is deallocated next to another un allocated chunk.
- To find memory easier see [multiboot.h](https://github.com/cstack/osdev/blob/m_is_master/multiboot.h), however I will implement my own for copyright reasons which will just use the multiboot structure
- Now that there is memory management objects will be able to use the "new" and "delete" methods, allowing for virtual deconstructions
- However, if there is no free memory then the function will return 0 which would work if in user space as memory pointer 0 is out of bounds. The OS cant throw an exception like c++ normally would as currently there is no exception handler
<!-- TOC --><a name="system-calls"></a>
### System Calls
See also [List of syscalls](https://x64.syscall.sh/)
- The kernel can load a binary executable from the hard drive and then store it in memory and then attach the memory pointer to task management
- When the operating system is unsecure from executables, they will be able to use assembly and such to talk to the hard drive and other devices, this can lead to malicious programs.
- To get around this the kernel has to be put in userspace once fully initiated. Userspace tells the device to ignore "outb" commands and other low level operations.
- However, programs do have to communicate with the hard drive and other devices so there has to be some, managed , way of doing so.
- So instead system calls are used. Basically the program will call a software interrupt (0x80) and then the kernel will read infomation from the AX and BX registers (AX and BX were set before the interrupt, AX is the system call number and BX is parameters)
- Therefore the implementation for this just involves setting up an interrupt handler that handles the interrupt 0x80, reads the AX  and BX registers and then runs the relative function
<!-- TOC --><a name="networking"></a>
# Networking
<!-- TOC --><a name="driver-am79c971"></a>
### Driver am79c971
See also [OSDev - PCNET](https://wiki.osdev.org/AMD_PCNET), [LowLevel - PCNET](http://www.lowlevel.eu/wiki/AMD_PCnet), [Logical and Physical Adresses](https://www.geeksforgeeks.org/logical-and-physical-address-in-operating-system/) [AMD_AM79C973](https://www.amd.com/system/files/TechDocs/20550.pdf)
- To get networking capability in the OS a driver for the virtualized network chip (am79c971) has to be written
- This device is a complicated one to write a driver for. However, it will follow the same implementation as the other drivers: a class derived from driver, a class derived from interrupt handler (interrupt number and port number can be gotten from PCI)
- The device takes a lot of code to initialize (similar to loads for setting VGA graphics mode).
- The networking device can have multiple send and receive buffers and for every m_buffer there needs to be an instance of a struct (which mainly has a pointer to that m_buffer)
- A problem with this though is that this is one of those devices that use some bits in the address for other purposes meaning it needs to be a multiple of 16 (similar to bar)
- So the m_buffer will be 2KB but then an additional 15 bytes are added on and then 4 bytes are removed. This allows for the multiple of 16 to be found
<!-- TOC --><a name="handler"></a>
### Handler
See also [Wikipedia - Ethernet Frame](https://en.wikipedia.org/wiki/Ethernet_frame)
- The driver written for the am79c971 can be utilized to receive and send data, however for the device to be useful a protocol handler has to be written.
- This protocol handler will look at the incoming data and decide what protocol should be used to interpret the data.
- The data that is received by the am79c971 will be call raw data
- The raw data is structured like this (Encoded in big endian):
- - First 6 bytes: Destination MAC Address (Should  be this devices MAC address or 0xFFFF if it is a broadcast (sent to all devices on the network))
- - Next 6 bytes: Source MAC Address (Who sent the data)
- - Next 2 bytes: EtherType (Indicate the protocol)
- - Next 46 - 1000 bytes: Data (Payload)
- - Last 4 bytes: Checksum (CRC)
- From this data the handler can then interpret the data e.g. discard if the MAC address isn't for this device or pass the data to the ARP handler if the EtherType is 0x0806 etc.
<!-- TOC --><a name="address-resolution-protocol"></a>
### Address Resolution Protocol
See also [Wikipedia - ARP](https://en.wikipedia.org/wiki/Address_Resolution_Protocol)
- The Address Resolution Protocol (ARP) is a communication protocol used for discovering the link layer address, such as a MAC address, associated with a given internet layer address, typically an IPv4 address.
- The ARP data block is as follows: (known as the payload in the raw-data)
- - First 2 Bytes: Hardware type (e.g. ethernet or WLAN)
- - Next 2 Bytes: Protocol (Will have already received this from handler 0x0806)
- - Next 1 Byte: m_size of hardware address (Mac Address = 6)
- - Next 1 Byte: m_size of protocol address (IP Address = 4)
- - Next 2 Bytes: Command (What does the sender what Question/Response)
- - Next 6 Bytes: Mac address of the sender
- - Next 4 Bytes: IP address of the sender
- - Next 6 Bytes: Mac address of the receiver
- - Next 4 Bytes: IP address of the receiver
- The reason the MAC address is repeated (as it is passed in the Handler's RAW data) is because there can be multiple devices between the sender and the receiver e.g. a WI-FI relay
- Therefore, the ARP packet contains the MAC address of the original device that sent the ARP request.
<!-- TOC --><a name="internet-protocol-v4"></a>
### Internet Protocol v4
See also [Wikipedia - IPv4](https://en.wikipedia.org/wiki/IPv4), [Wikipedia - IPv4 Header](https://en.wikipedia.org/wiki/IPv4_header)
- The Internet Protocol (IP) is the principal communications protocol in the Internet protocol suite for relaying datagrams across network boundaries. Its routing function enables internetworking, and essentially establishes the Internet.
- The IP data block is as follows: (known as the payload in the raw-data)
- - First 1 Byte: Version (4)
- - Next 1 Byte: Header Length (Number of 32 bit words in the header)
- - Next 1 Byte: Type of Service (Used for Quality of Service)
- - Next 2 Bytes: Total Length (Header + Data)
- - Next 2 Bytes: Identification (Used for fragmentation, when a packet is too big for the ethernet frame and has to be split up)
- - Next 2 Bytes: Flags (Used for fragmentation)
- - Next 1 Byte: Time to live (How many hops the packet can make)
- - Next 1 Byte: Protocol (What protocol is the data)
- - Next 2 Bytes: Header Checksum (CRC)
- - Next 4 Bytes: Source IP Address (Who sent the data)
- - Next 4 Bytes: Destination IP Address (Who is the data for)
- - Next 4 Bytes: Options (Optional, if header length is greater than 5)
- The time to live is used to prevent infinite loops. If a packet is sent to a device that is not on the network, the packet will be sent back to the sender. If the time to live is 0, the packet will be discarded.
- It is also used in traceroute to see how many hops it takes to get to the destination. Starts by sending a packet with a time to live of 1, if it gets to the destination it will be discarded. If it gets to a device that is not on the network, the packet will be sent back to the sender. The sender will then send a packet with a time to live of 2 and so on.
- The checksum is set by the sender and is used to check if the data has been corrupted in transit.
- The protocol is used to determine what protocol the data is for. e.g. 0x06 is TCP, 0x11 is UDP
- The options are used to add extra information to the header. e.g. the maximum segment size (MSS) is used to tell the receiver how big the data can be. However, this is not used in most protocols today.
- The header length is used to determine how many 32 bit words are in the header. This is used to determine where the data starts.
- The total length is used to determine how many bytes are in the packet. This is used to determine where the data ends.
- The identification is used to determine if the packet is part of a fragmented packet. If the identification is the same as a previous packet, the data is added to the previous packet.
- The flags are used to determine if the packet is part of a fragmented packet. If the flags are 0x02, the packet is the last packet in the fragmented packet.
- The type of service is used to determine the quality of service. e.g. 0x10 is low delay, 0x08 is high throughput, 0x04 is high reliability.
- The version is used to determine the version of the protocol. e.g. 4 is IPv4 and 6 is IPv6
- The source IP address is used to determine who sent the packet. This is used to determine if the packet is for this device. If it is not, the packet is discarded.
- The destination IP address is used to determine who the packet is for. This is used to determine if the packet is for this device. If it is not, the packet is discarded. If it is, the packet is passed to the next protocol handler.
- The data is the data that is being sent. This is passed to the next protocol handler.
<!-- TOC --><a name="internet-control-message-protocol"></a>
### Internet Control Message Protocol
See also [Wikipedia - ICMP](https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol)
- The Internet Control Message Protocol (ICMP) is a protocol used by network devices, including routers, to send error messages and operational information indicating, for example, that a requested service is not available or that a host or router could not be reached.
- ICMP is an integral part of the Internet protocol suite. It is used by network devices, including routers, to send error messages and operational information indicating, for example, that a requested service is not available or that a host or router could not be reached. A primary use is in error reporting, where a host sends an ICMP message to a router when it is unable to reach another host. The router can then send error messages back to the source host, indicating the nature of the problem.
- The ICMP data block is as follows: (known as the payload in the raw-data)
- - First 1 Byte: Type (What type of message is being sent)
- - Next 1 Byte: Code (What code is being sent)
- - Next 2 Bytes: Checksum (CRC)
- - Next 4 Bytes: Data (Optional)
- Message Types: See [Wikipedia - ICMP Message Types](https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Control_messages)
<!-- TOC --><a name="user-datagram-protocol"></a>
### User Datagram Protocol
See also [Wikipedia - UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol)
- The User Datagram Protocol (UDP) is one of the core members of the Internet protocol suite. It is a connectionless protocol, meaning that UDP datagrams are not guaranteed to arrive in the same order in which they were sent, and may be duplicated or lost entirely.
- UDP is used for time-sensitive applications, such as voice over IP (VoIP), streaming media, and real-time multiplayer games.
- The reason it is used for time sensitive applications is because it does not require the host to acknowledge receipt of the data. This means that if the data is lost it will not be resent and the application will not know. This is fine for time sensitive applications as the data is not needed to be received. In a real time application, if the data is lost then maybe a frame will be skipped but the application will not crash and the user generally will not notice.
- It is also used for DNS lookups, and for low-latency and loss-tolerating applications, such as real-time audio and video streaming.
- UDP is a simple protocol, with no handshaking or acknowledgment of data packets. It is used in conjunction with the Internet Protocol (IP) and the Internet Control Message Protocol (ICMP) to provide a best-effort datagram service.
- The UDP data block is as follows: (known as the payload in the raw-data)
- - First 2 Bytes: Source Port (Port that the data is coming from)
- - Next 2 Bytes: Destination Port (Port that the data is going to)
- - Next 2 Bytes: Length (Length of the UDP data block)
- - Next 2 Bytes: Checksum (CRC)
- - Next 0 - 65507 Bytes: Data (Payload)
- The reason the ports are repeated (as it is passed in the Handler's RAW data) is because there can be multiple devices between the sender and the receiver e.g. a WI-FI relay
- Therefore, the UDP packet contains the port of the original device that sent the UDP request.
- The UDP is just a multiplexer and demultiplexer, similar to CPU in data send / receive section. This means that it does not care what the data is, it just passes it on.
<!-- TOC --><a name="transmission-control-protocol"></a>
### Transmission Control Protocol
See also [Wikipedia - TCP](https://en.wikipedia.org/wiki/Transmission_Control_Protocol)
- The Transmission Control Protocol (TCP) is one of the main protocols of the Internet protocol suite. It originated in the initial network implementation in which it complemented the Internet Protocol. It provides reliable, ordered, and error-checked delivery of a stream of octets (bytes) between applications running on hosts communicating via an IP network. Major internet applications such as the World Wide Web, email, remote administration, and file transfer rely on TCP. Applications that do not require reliable data stream service may use the User Datagram Protocol (UDP), which provides a connectionless datagram service that emphasizes reduced latency over reliability.
- TCP is a connection-oriented protocol, which means that two computers must m_first_memory_chunk establish a connection before exchanging data.
- The reason it is connection oriented is that it requires the host to acknowledge receipt of the data. This means that if the data is lost it will be resent and the application will know. This is fine for applications that require the data to be received.
- TCP is a reliable protocol, which means that it will resend any lost packets. This is done by using a sequence number and an acknowledgement number. The sequence number is the number of the m_first_memory_chunk byte in the data. The acknowledgement number is the number of the next byte that is expected. If the acknowledgement number is not the same as the sequence number, the data is resent.
- The TCP data block is as follows: (known as the payload in the raw-data)
- - First 2 Bytes: Source Port (Port that the data is coming from)
- - Next 2 Bytes: Destination Port (Port that the data is going to)
- - Next 4 Bytes: Sequence Number (Number of the m_first_memory_chunk byte in the data)
- - Next 4 Bytes: Acknowledgement Number (Number of the next byte that is expected)
- - Next 1 Byte: Header Length (Length of the TCP header)
- - Next 1 Byte: Flags (Flags that are set)
- - Next 2 Bytes: Window m_size (Window is the amount of data that can be sent before an acknowledgement is required)
- - Next 2 Bytes: Checksum (CRC)
- - Next 2 Bytes: Urgent Pointer (Pointer to the urgent data)
- - Next 0 - 65507 Bytes: Data (Payload)
- For security, the sender and receiver agree on an m_offset. This is the number of bytes that are added to the sequence number. This means that the sequence number is not the same as the data. This is to prevent people from being able to guess the sequence number and therefore the data, intercepting it and sending malicious data whilst impersonating the sender.
- To disconnect, the sender sends a FIN packet. The receiver then sends an ACK packet. The sender then sends an ACK packet. The receiver then sends a FIN packet. The sender then sends an ACK packet. The receiver then sends an ACK packet.
- To create the checksum the TCP header and the data is added together. The checksum is then added to the header. The checksum is then calculated again. If the checksum is the same then the data is m_valid.
- It is calculated by adding the 16-bit words together. If the result is greater than 16 bits, then the carry is added to the result. The result is then inverted.
- The command flags are what are sent in the TCP header. The flags are as follows:
- - FIN: The connection is finished (1)
- - SYN: Synchronize sequence numbers (2)
- - RST: Reset the connection (4)
- - PSH: Push function (8)
- - ACK: Acknowledgement (16)
- - URG: Urgent (32)
- - ECE: ECN-Echo (64)
- - CWR: Congestion Window Reduced (128)
- These flags are used to control the flow of data. For example, the SYN flag is used to start the connection. The ACK flag is used to acknowledge the data. The FIN flag is used to end the connection.
<!-- TOC --><a name="hyper-text-transfer-protocol"></a>
### Hyper Text Transfer Protocol
See also [Wikipedia - HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
TBC
<!-- TOC --><a name="domain-name-system"></a>
### Domain Name System
See also [Wikipedia - DNS](https://en.wikipedia.org/wiki/Domain_Name_System)
TBC
<!-- TOC --><a name="dynamic-host-configuration-protocol"></a>
### Dynamic Host Configuration Protocol
See also [Wikipedia - DHCP](https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol)
TBC
<!-- TOC --><a name="secure-shell"></a>
### Secure Shell
See also [Wikipedia - SSH](https://en.wikipedia.org/wiki/Secure_Shell)
TBC
<!-- TOC --><a name="file-transfer-protocol"></a>
### File Transfer Protocol
See also [Wikipedia - FTP](https://en.wikipedia.org/wiki/File_Transfer_Protocol)
TBC
<!-- TOC --><a name="simple-mail-transfer-protocol"></a>
### Simple Mail Transfer Protocol
See also [Wikipedia - SMTP](https://en.wikipedia.org/wiki/Simple_Mail_Transfer_Protocol)
TBC

<!-- TOC --><a name="open-systems-interconnection-model"></a>
### Open Systems Interconnection Model
See also [Wikipedia - OSI Model](https://en.wikipedia.org/wiki/OSI_model)
- The Open Systems Interconnection (OSI) model is a conceptual model that characterises and standardises the communication functions of a telecommunication or computing system without regard to its underlying internal structure and technology.
- OSI is a reference model for how information is passed between applications and computer systems.
- It is structured in a layered approach, with each layer performing a specific function.
- The layers are as follows:
- - Application Layer (Handled in the application)
- - Presentation Layer (Handled in the application)
- - Session Layer (Handled in the application)
- - Transport Layer (TCP, UDP)
- - Network Layer (Ipv4)
- - Data Link Layer (Ether frame)
- - Physical Layer  (am79c971)
- It is important to network because it allows for the different layers to be implemented in different ways. For example, the physical layer can be implemented using a wireless connection or a wired connection. The network layer can be implemented using a router or a switch. The transport layer can be implemented using TCP or UDP. The application layer can be implemented using HTTP or FTP.
- The operating system will implement the layers above the network layer (Application, Presentation, Session, Transport) and the network device will implement the layers below the network layer (Data Link, Physical)
- The reason for this is that the network device is the only thing that can communicate with the physical layer and the network layer. The operating system can only communicate with the application layer and the transport layer.
