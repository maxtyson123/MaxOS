# Hardware Communications
- When you press a key on the keyboard a signal will go to the programmable interrupt controller (PIC)
- PIC will just ignore that, so to receive the key signal, tell PIC not to ignore it
- This is done by sending some data to the PIC
- This means there has the to be a method of talking to the hardware (send data out / receive data in)
- CPU has a multiplexer and a de-multiplexer which is connected to different hardware which is what physically sends the data
- The PIC (a piece of hardware) has the 32 (0x20), so the data for the PIC has to be sent port 32 via the multiplexer
- Using assembler, we can send data using:
```bash
 outb(portNumber, data)
 outb(0x20, 0x1) //Example using PIC (port 32) and the data 1
```
- However, as the os is coded in C++, assembly is not a good idea so instead there is a port object for each one
- This port object will be of 8bit, 16bit etc.. for the bandwidths 
- See "port.h"