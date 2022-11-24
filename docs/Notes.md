NOTE: method is the same is function (or at least in my wording here)

# Hardware Communication
Here are some notes on how the communication with hardware works, this is used for the keyboard and mouse communication and setting up other devices, e.g. GPU
### Data Send / Receive
This relates to "port.cpp", directly used by "interrupts.cpp, mouse.cpp, keyboard.cpp"
- When you press a key on the keyboard a signal will go to the programmable interrupt controller (PIC)
- By default, the PIC is set ignore that, so to receive the key signal tell PIC not to ignore it
- This is done by sending some data to the PIC
- This means there has the to be a method of talking to the hardware (send data out / receive data in)
- CPU has a multiplexer and a de-multiplexer which is connected to different hardware which is what physically sends the data
- The PIC (a piece of hardware) has the port number 32 (0x20), so the data for the PIC has to be sent port 32 via the multiplexer
- Using assembler, data can be set:
```bash
 outb(portNumber, data)
 outb(0x20, 0x1) //Example using PIC (port 32) and the data 1
```
- To initialize the PIC  ICWs (Initialization Control Words) must be sent
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
### Base Address Registers
- The solution for having multiple PCI devices (two GPUS, two screens etc…) is to have a Base Address Registers (BAR)
- Base address registers are registers in the PCI configuration space, which in a more simple way is just storing the address of a PCI device in memory
- There are two types of BARs, one is  the I/O BAR, this is for devices that communicate bit by bit e.g. mouse where the position is read from the port each interrupt 
- The I/O base address register is 4 bytes long:
- - Lowest bit:            Type Of register: (I/O bit = 1)
- - Second-lowest bit:     Reserved 
- - Bits 3 - 16:           Port Number (MUST BE MULTIPLE OF 4)
- Memory mapping is another type of BAR, this one is where the device takes x memory location, and to communicate with it just write/read from that location in memory. This is better performance-wise as the CPU can do other tasks while that is updating in the background
- The Memory Mapping BAR is 4 bytes long:
- - Lowest Bit:  Type Of register: (MemMap bit = 0)
- - Second and third: (00 = 32bit BAR) or (01 = 20bit BAR) or (10 = 64bit BAR)
- - Fourth bit: Prefetch-able bit (e.g. reading from a hard drive in advance because its estimated that the program will need that soon)
- - Last Twelve: Ram Address (MUST BE MULTIPLE OF 4)
### Hard-drives
- History of hard drives: 20 years ago there used to be IDE (Integrated Device Electronics) that was later continued on to ATA (Advanced Technology Attachment) that has now been moved onto what is used to day SATA (Serial Advanced Technology Attachment) 
- As hard drives have been around for a long time, they are well documented and easy to implement
- SATA devices are almost always compatible to ATA or AHCI (Advanced Host Controller Interface)
- Using the PCI controller, SATA hard drives can be found on class id 0x01 and subclass id 0x06
- There are two different ways of accessing a hard-drive. The first one is PIO (Programed Input Output) which is relatively simple to implement but rather slow (maxs out at 16 M/Bs)
- The second and better way would be DMA (Direct Memory Access) , which simply just writes the data to a memory location and sends an interrupt once finished
- Although it is better to read from the PCI devices list, the ports can just be hard coded 
- For the Implementation of PIO:
- - PIO Has 28Bit Mode and 48Bit Mode
- - These modes just specify how many bits are transferred to the device to request a sector (Sectors are 512 bytes)
- - 28Bits supports hard drives/files upto 4GB whereas 48Bits support hard drives/files upto 8PB
- Previously to communicate with data on the hard drive the OS had to know the CHS (Cylinder, Head, Sector)  address. As hard drive developed and there were no longer spinning disks and such (e.g. SSDs) a new way of comincating was invented: LBA.
- LBA stands for Logical Block Address which basically just returns a sector with a given number


# Graphics
Here are the notes on graphics for the operating system. (May need to read hardware communication first)
### Graphics Mode {VGA}
- To write pixels and such to the screen for a GUI the graphics card needs to be put into graphics mode (grub puts it into text mode by default)
- There are two ways of doing this, via the BIOS 0x13 Interrupt or by directly telling the GPU.
- As the BIOS interrupt 0x13 is only for 16Bit operating system (MaxOS is 32bit) graphics mode would have to be set via the GPU
- Setting graphics mode via the GPU is incredibly complicated therefore the script ['modes.c' by Chris Giese](https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c) is used
- To set the graphics mode via the GPU different sets of values have to be sent to different places, for example a 320px wide by 200px height x 8-bit colour depth graphics mode would have to be set by sending these values:
- - Misc Port: 0x63,
- - Sequencer Port:  0x03, 0x01, 0x0F, 0x00, 0x0E,
- - (CRTC) Cathode ray tube controller Port:  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF,
- - (GC) Graphics controller Port: 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
- - (AC) Attribute Controller: 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00
- However, communicating with these ports aren't that easy as there is an index port (to tell where the data should be put) and a data port (push the data)
- And on the CRTC port there is special indexes and data that has to be set for it to unlocked before writing actual data (the reason it has to be unlocked is that if incorrect data is the hardware could be damaged)
- Similarly, the AC port has to be reset before each index & data write.
- The VGA class that was written should have a class above that call graphicsContext which would define methods such as putPixel(...), drawRect(...), drawLine(...), drawCircle(...) and sub-classes such as color.
- The draw functions should rely on the putPixel function therefore in different graphics modes that are derived from that graphicsContext class can be supported.

### GUI - Framework
See also: Bresenham line algorithm (use for later)
- The GUI framework will have a base class called Widget, which should have a draw method. This draw method would get a graphicsContext and draw itself onto of it using the graphicContext's draw methods.
- Thees widgets will have parents and such, therefore their draw positions will be relative, meaning there has to be a method to get the absolute position.
- Widgets will also have a width and height, and a method to check if a co-ordinate is inside itself (used for mouse handling and such)
- To handle mouse input, the widget will use a mouse event handler and handle the events onMouseDown, onMouseMove, OnMouseUp. However, the mouse handler won't be on the widget itself, rather it would be on the desktop.
- This is because the mouse movement is reported in relative-ness not absolute position (desktop would store mouse position and update it based on the movement, the object can just query the x,y pos from the desktop).
- A subclass of the widget would be a composite class, which would contain an array of child widgets, and it would pass methods on to the children (e.g. the child gets drawn last so its on top)
- To get which widget to preform the keyEvent or mouseEvent the desktop will have a method of getting the focused widget
- The desktop will work like the composite widget, but will have to override the mouseHandler class as mouse passes movement in relative to last position (moved x  pixels) so the desktop will have to translate the restiveness into absolute positions
- For better performance, instead of re-drawing the screen every time just draw it once and then when a gui object changes (e.g. moving a window) memory the object to the new state and then redraw any invalid parts (invalid is where places where there used to be the gui object)
- To draw text the "modes.c" has various 8x8 Bitmap fonts. Using these fonts a Text widget can be made that gets the characters of a string from the font and then print all the pixels in that character.  

# System Stuff
### Multitasking
See also [Stack](https://wiki.osdev.org/Stack), **Note this needs better explaining
- In the RAM there is different places for different species of code. The processor jumps around executing different parts. Somewhere in the RAM there is a STACK where stuff is being pushed a popped
- Before Multitasking: A timer interrupt occurs and what happens is the processor pushes some registers on the stack (e.g. pointer to where it was executing before the interrupt). The processor then goes into the interrupt manager (see interrupts.cpp) and executes the handler for that interrupt. Once the handler has finished executing the processor then carry's on what it was executing before (hence why the pointer was pushed to the stack).
- However, to do multithreading the current way of executing in the OS needs to change. First the OS will take another portion of the RAM, reserve it for multitasking, and make its own stack there for every task.
- Therefore, when a task is created a memory region is made. Information for the processor is then explicitly writen into segments in that region e.g. the instruction pointer at the entry point
- Previously: the interstubs.s file pushed the current processor values (interstubs line 64: save the registers) and then pushed the stack pointer (interstubs line 80: Invoke C++ handlers) and then load register again
- (Carrying on "Previously") The handler function would have the pointer to the stack (passed as var: esp). The return value of handle interrupt is then written into esp and then executed by processor. 
- Now with multitasking the return value for the handle interrupt method would be changed to point to the top of the new task's stack.
- To sum up: 
- - The kernel will be executing method "X" and receive an interrupt
- - kernel will execute the interrupt handler
- - The interrupt handler will then return a pointer to the new task
- - Kernel will then execute the new task instead of executing method "X"
- To schedule processes fairly, a round-robin scheduler generally employs time-sharing, giving each job a time slot or quantum (its allowance of CPU time), and interrupting the job if it is not completed by then. The job is resumed next time a time slot is assigned to that task. If the task terminates or changes its state to waiting during its attributed time quantum, the scheduler selects the first task in the ready queue to execute
### Threads
See also: [Thread](https://wiki.osdev.org/Thread)
- A thread is a lightweight process. It is a sequence of instructions within a program that can be executed independently of other code. Threads can be created and managed by a thread library.
- Threads are often used in parallel computing, where they can be used to perform multiple tasks simultaneously. Threads are also used in multimedia, where they can be used to perform different tasks in real time. For example, a thread can be used to play a sound while another thread is used to display a graphic.
- Threads are also used in operating systems, where they can be used to perform different tasks simultaneously. For example, a thread can be used to play a sound while another thread is used to display a graphic.
- The main difference between a thread and a process is that threads share the same address space, while processes have separate address spaces. This means that threads can share data and code, while processes cannot. This also means that threads are faster than processes, because they do not need to be loaded into memory.
- To implement threads in the OS, the kernel will have to implement a thread class. This class will have a method to create a thread, and a method to execute the thread. The thread class will also have a method to get the current thread.
- The thread class will also have a method to get the current thread. This method will be used to get the current thread, and then the thread will be able to access its own stack.
- Yeilding is when a thread gives up its time slice and allows another thread to run. This is done by calling the yeild method on the thread class.
- Processes and threads are different. A process is a program in execution. A thread is a component of a process. A process can have multiple threads. Threads are often used to implement concurrency within a process. Threads are also used to implement parallelism, where multiple processes can run simultaneously on a multi-core processor.
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
- To find memory easier see [multiboot.h](https://github.com/cstack/osdev/blob/master/multiboot.h), however I will implement my own for copyright reasons which will just use the multiboot structure
- Now that there is memory management objects will be able to use the "new" and "delete" methods, allowing for virtual deconstructions
- However, if there is no free memory then the function will return 0 which would work if in user space as memory pointer 0 is out of bounds. The OS cant throw an exception like c++ normally would as currently there is no exception handler
### System Calls
See also [List of syscalls](https://faculty.nps.edu/cseagle/assembly/sys_call.html)
- The kernel can load a binary executable from the hard drive and then store it in memory and then attach the memory pointer to task management
- When the operating system is unsecure from executables, they will be able to use assembly and such to talk to the hard drive and other devices, this can lead to malicious programs.
- To get around this the kernel has to be put in userspace once fully initiated. Userspace tells the device to ignore "outb" commands and other low level operations. 
- However, programs do have to communicate with the hard drive and other devices so there has to be some, managed , way of doing so.
- So instead system calls are used. Basically the program will call a software interrupt (0x80) and then the kernel will read infomation from the AX and BX registers (AX and BX were set before the interrupt, AX is the system call number and BX is parameters) 
- There for the implementation for this just involves setting up an interrupt handler that handles the interrupt 0x80, reads the AX  and BX registers and then runs the relative function
# Networking
### Driver
See also [OSDev - PCNET](https://wiki.osdev.org/AMD_PCNET), [LowLevel - PCNET](http://www.lowlevel.eu/wiki/AMD_PCnet), [Logical and Physical Adresses](https://www.geeksforgeeks.org/logical-and-physical-address-in-operating-system/) [AMD_AM79C973](https://www.amd.com/system/files/TechDocs/20550.pdf)
- To get networking capability in the OS a driver for the virtualized network chip (am79c971) has to be written
- This device is a complicated one to write a driver for. However, it will follow the same implementation as the other drivers: a class derived from driver, a class derived from interrupt handler (interrupt number and port number can be gotten from PCI)
- The device takes a lot of code to initialize (similar to loads for setting VGA graphics mode).
- The networking device can have multiple send and receive buffers and for every buffer there needs to be an instance of a struct (which mainly has a pointer to that buffer)
- A problem with this though is that this is one of those devices that use some bits in the address for other purposes meaning it needs to be a multiple of 16 (similar to bar)
- So the buffer will be 2KB but then an additional 15 bytes are added on and then 4 bytes are removed. This allows for the multiple of 16 to be found
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
- From this data the handler can then interpret the data e.g. discard if the MAC adress isnt for this device or pass the data to the ARP handler if the EtherType is 0x0806 etc.
### Address Resolution Protocol
See also [Wikipedia - ARP](https://en.wikipedia.org/wiki/Address_Resolution_Protocol)
- The Address Resolution Protocol (ARP) is a communication protocol used for discovering the link layer address, such as a MAC address, associated with a given internet layer address, typically an IPv4 address.
- The ARP data block is as follows: (known as the payload in the raw-data)
- - First 2 Bytes: Hardware type (e.g. ethernet or WLAN)
- - Next 2 Bytes: Protocol (Will have already received this from handler 0x0806)
- - Next 1 Byte: Size of hardware address (Mac Address = 6)
- - Next 1 Byte: Size of protocol address (IP Address = 4)
- - Next 2 Bytes: Command (What does the sender what Question/Response)
- - Next 6 Bytes: Mac address of the sender
- - Next 4 Bytes: IP address of the sender
- - Next 6 Bytes: Mac address of the receiver
- - Next 4 Bytes: IP address of the receiver
- The reason the MAC address is repeated (as it is passed in the Handler's RAW data) is because there can be multiple devices between the sender and the receiver e.g. a WI-FI relay 
- Therefore, the ARP packet contains the MAC address of the original device that sent the ARP request.
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