# Hardware Communications
### Data Send / Receive
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
- To intialize the PIC  ICWs (Initialization Control Words) must be sent
### Interrupts
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
- From the assembily code a c++ function can then be called (simmilar to calling kernelMain in loader.s)