
# Hardware Communication
Here are some notes on how the comincation with hardware works, this is used for the keyboard and mouse communication 
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
  - Segment: tells the processor to switch a certain segment before executing handler, (this is for when the processor is executing code in the usersapce, the handler is in the kernelspace and the processor needs to switch to the kernel segment)
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
- The soultion for having multiple PCI devices (two GPUS, two screens etc..) is to have a Base Adress Registers (BAR)
- Base adress registers are registers in the PCI configuration space, which in a more simple way is just storing the address of a PCI device in memory
- There are two types of BARs, one is  the I/O BAR, this is for devices that communicate bit by bit e.g. mouse where the position is read from the port each interrupt 
- The I/O base address register is 4 bytes long:
- - Lowest bit:            Type Of register: (I/O bit = 1)
- - Second-lowest bit:     Reserved 
- - Bits 3 - 16:           Port Number (MUST BE MULTIPLE OF 4)
- Memory mapping is another type of BAR, this one is where the device takes x memory location, and to communicate with it just write/read from that location in memory. This is better preformace-wise as the CPU can do other tasks while that is updating in the background
- The Memory Mapping BAR is 4 bytes long:
- - Lowest Bit:  Type Of register: (MemMap bit = 0)
- - Second and third: (00 = 32bit BAR) or (01 = 20bit BAR) or (10 = 64bit BAR)
- - Fourth bit: Prefetch-able bit (eg. reading from a hard drive inadvace becuase its estamated that the program will need that soon)
- - Last Twelve: Port Number (MUST BE MULTIPLE OF 4)
