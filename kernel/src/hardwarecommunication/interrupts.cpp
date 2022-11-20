//
// Created by 98max on 7/10/2022.
//

#include <hardwarecommunication/interrupts.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::system;



void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration
char printfInt( long num );                     //Forward declaration

///__Handler__

InterruptHandler::InterruptHandler(uint8_t interrupNumber, InterruptManager *interruptManager){
    //Store vals given
    this->interrupNumber = interrupNumber;
    this->interruptManager = interruptManager;
    //Put itself into handlers array
    interruptManager->handlers[interrupNumber] = this;
}
InterruptHandler::~InterruptHandler(){
    //Remove self from handlers array
    if(interruptManager->handlers[interrupNumber] == this){
        interruptManager->handlers[interrupNumber] = 0;
    }
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp){
    //Standard for the handlers, specifics will be created when init each type
    return esp;
}


///__Manger__

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

InterruptManager* InterruptManager::ActiveInterruptManager = 0;

/**
 * @details This function is used to set an entry in the IDT
 * @param interrupt  Interrupt number
 * @param CodeSegment  Code segment
 * @param handler  Interrupt Handler
 * @param DescriptorPrivilegeLevel Descriptor Privilege Level
 * @param DescriptorType  Descriptor Type
 */
void InterruptManager::SetInterruptDescriptorTableEntry(uint8_t interrupt,
                                                        uint16_t CodeSegment,
                                                        void (*handler)(),
                                                        uint8_t DescriptorPrivilegeLevel,
                                                        uint8_t DescriptorType)
{
    // address of pointer to code segment (relative to global descriptor table)
    // and address of the handler (relative to segment)
    interruptDescriptorTable[interrupt].handlerAddressLowBits = ((uint32_t) handler) & 0xFFFF;
    interruptDescriptorTable[interrupt].handlerAddressHighBits = (((uint32_t) handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interrupt].gdt_codeSegmentSelector = CodeSegment;

    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interrupt].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType; //Combine constant with descriptor type and level. The level is shifted by 5 and only the last 3 bits are needed
    interruptDescriptorTable[interrupt].reserved = 0;                                                                       //Nothing needs to be reserved
}


InterruptManager::InterruptManager(uint16_t hardwareInterruptOffset, system::GlobalDescriptorTable* globalDescriptorTable,ThreadManager* threadManager)
        : programmableInterruptControllerMasterCommandPort(0x20),
          programmableInterruptControllerMasterDataPort(0x21),
          programmableInterruptControllerSlaveCommandPort(0xA0),
          programmableInterruptControllerSlaveDataPort(0xA1)
{
    this->threadManager = threadManager;
    this->hardwareInterruptOffset = hardwareInterruptOffset;
    uint32_t CodeSegment = globalDescriptorTable->CodeSegmentSelector();

    //Set all the entry's to Ignore so that the ones we don't specify aren't run as there won't be a handler for these and therefore would have caused a protection error
    const uint8_t IDT_INTERRUPT_GATE = 0xE;
    for(uint8_t i = 255; i > 0; --i)
    {
        SetInterruptDescriptorTableEntry(i, CodeSegment, &InterruptIgnore, 0, IDT_INTERRUPT_GATE);  //Set to ignore
        handlers[i] = 0;                                                                                                                         //Set to no handler
    }
    SetInterruptDescriptorTableEntry(0, CodeSegment, &InterruptIgnore, 0, IDT_INTERRUPT_GATE);      //Set to ignore (for first in array)
    handlers[0] = 0;                                                                                                                             //Set to no handler (for first in array)



    //Set Up the base interrupts
    SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);   //Division by zero
    SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);   //Single-step interrupt (see trap flag)
    SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);   //NMI
    SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);   //Breakpoint (which benefits from the shorter 0xCC encoding of INT 3)
    SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);   //Overflow
    SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);   //Bound Range Exceeded
    SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);   //Invalid Opcode
    SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);   //Coprocessor not available
    SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);   //Double Fault
    SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);   //Coprocessor Segment Overrun (386 or earlier only)
    SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);   //Invalid Task State Segment
    SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);   //Segment not present
    SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);   //Stack Segment Fault
    SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);   //General Protection Fault
    SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);   //Page Fault
    SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);   //x87 Floating Point Exception
    SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);   //Alignment Check
    SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);   //Machine Check
    SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);   //SIMD Floating-Point Exception
    SetInterruptDescriptorTableEntry(0x14, CodeSegment, &HandleException0x14, 0, IDT_INTERRUPT_GATE);   //Virtualization Exception
    SetInterruptDescriptorTableEntry(0x15, CodeSegment, &HandleException0x15, 0, IDT_INTERRUPT_GATE);   //Control Protection Exception
    SetInterruptDescriptorTableEntry(0x16, CodeSegment, &HandleException0x16, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x17, CodeSegment, &HandleException0x17, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x18, CodeSegment, &HandleException0x18, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x19, CodeSegment, &HandleException0x19, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x1A, CodeSegment, &HandleException0x1A, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x1B, CodeSegment, &HandleException0x1B, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x1C, CodeSegment, &HandleException0x1C, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x1D, CodeSegment, &HandleException0x1D, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x1E, CodeSegment, &HandleException0x1E, 0, IDT_INTERRUPT_GATE);   //reserved
    SetInterruptDescriptorTableEntry(0x1F, CodeSegment, &HandleException0x1F, 0, IDT_INTERRUPT_GATE);   //reserved


    //Set up the hardware interrupts (offest by 0x20) //Ranges 0x20 - 0x30
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);  //0x20 - Default PIC interval   / Timer
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x01, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);  //0x21 - Keyboard
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x02, CodeSegment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);  //0x22 - Cascade (used internally by the two PICs. never raised)
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x03, CodeSegment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);  //0x23 - COM2, COM4
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x04, CodeSegment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);  //0x24 - COM1, COM3
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x05, CodeSegment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);  //0x25 - LPT2, LPT4
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x06, CodeSegment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);  //0x26 - LPT1
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x07, CodeSegment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);  //0x27 - Floppy Disk
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x08, CodeSegment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);  //0x28 - CMOS Real Time Clock
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x09, CodeSegment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);  //0x29 - Free for peripherals / legacy SCSI / NIC
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0A, CodeSegment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);  //0x2A - Free for peripherals / SCSI / NIC
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0B, CodeSegment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);  //0x2B - Free for peripherals / SCSI / NIC
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);  //0x0C - Mouse
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0D, CodeSegment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);  //0x2D - FPU / Coprocessor / Inter-processor
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0E, CodeSegment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);  //0x2E - Primary ATA Hard Disk
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0F, CodeSegment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);  //0x2F - Secondary ATA Hard Disk

    SetInterruptDescriptorTableEntry(                          0x80, CodeSegment, &HandleInterruptRequest0x80, 0, IDT_INTERRUPT_GATE);  //0x80 - Sys calls

    //Send Initialization Control Words
    programmableInterruptControllerMasterCommandPort.Write(0x11);
    programmableInterruptControllerSlaveCommandPort.Write(0x11);

    //Because the CPU uses interrupt 1 etc. , tell the PIC to add 0x20 to it (this is the remapping of hardware)
    programmableInterruptControllerMasterDataPort.Write(hardwareInterruptOffset);
    programmableInterruptControllerSlaveDataPort.Write(hardwareInterruptOffset+8);

    //Tell PICs their roles
    programmableInterruptControllerMasterDataPort.Write(0x04);  //Master
    programmableInterruptControllerSlaveDataPort.Write(0x02);   //Slave

    //Tell PICS that they are in 8086 mode
    programmableInterruptControllerMasterDataPort.Write(0x01);
    programmableInterruptControllerSlaveDataPort.Write(0x01);

    //This represents the value in the Interrupt Mask Register (IMR), This enables interrupts
    programmableInterruptControllerMasterDataPort.Write(0x00);
    programmableInterruptControllerSlaveDataPort.Write(0x00);

    //Tell the processor to use the IDT
    InterruptDescriptorTablePointer idt_pointer;
    idt_pointer.size  = 256*sizeof(GateDescriptor) - 1;
    idt_pointer.base  = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt_pointer));
};

InterruptManager::~InterruptManager()
{
    Deactivate();
}

/**
 * @details This function activates the interrupt manager
 */
void InterruptManager::Activate() {

    if(ActiveInterruptManager != 0){                //There shouldn't be another interrupt manager, but for saftey delete anyother ones. This is becuase the processor only has 1 IDT
        ActiveInterruptManager->Deactivate();
    }

    ActiveInterruptManager = this;
    asm("sti"); //sti = start interrupts

}

/**
 * @details This function deactivates the interrupt manager
 */
void InterruptManager::Deactivate()
{
    if(ActiveInterruptManager == this){         //Only if the active InterruptManger
        ActiveInterruptManager = 0;
        asm("cli");                             //cli = clear interrupts
    }


}

/**
 * @details This function passes the interrupt request on to the active interrupt manager
 * @param interruptNumber The interrupt number
 * @param esp The stack pointer
 * @return The stack pointer
 */
uint32_t InterruptManager::HandleInterrupt(uint8_t interrupt, uint32_t esp)
{

    if(ActiveInterruptManager != 0){
        return ActiveInterruptManager->DoHandleInterrupt(interrupt, esp);       //Handle the interrupt in OOP mode instead of Static
    }

    return esp;
}

/**
 * @details This function handles the interrupt request
 * @param interruptNumber The interrupt number
 * @param esp The stack pointer
 * @return The stack pointer
 */
uint32_t InterruptManager::DoHandleInterrupt(uint8_t interrupt, uint32_t esp)
{
    if(handlers[interrupt]!= 0){                                //If it has a handler for it
        esp = handlers[interrupt]->HandleInterrupt(esp);        //Run the handler
    }else{
        if(interrupt != 0x20){   //If not the timer interrupt

            switch (interrupt) {

                case 0x00:  //Divide by zero
                    printf("[ERROR] Divide by zero  (int 0x00)");
                    break;

                case 0x01: //Single step
                    printf("[ERROR] Single step (int 0x01)");
                    break;

                case 0x02: //Non maskable interrupt
                    printf("[ERROR] Non maskable interrupt (int 0x02)");
                    break;

                case 0x03: //Breakpoint
                    printf("[ERROR] Breakpoint (int 0x03)");
                    break;

                case 0x04: //Overflow
                    printf("[ERROR] Overflow (int 0x04)");
                    break;

                case 0x05: //Bounds check
                    printf("[ERROR] Bounds check  (int 0x05)");
                    break;

                case 0x06: //Invalid opcode
                    printf("[ERROR] Invalid opcode  (int 0x06)");
                    break;

                case 0x07: //Coprocessor not available
                    printf("[ERROR] Coprocessor not available  (int 0x07)");
                    break;

                case 0x08: //Double fault
                    printf("[ERROR] Double fault (int 0x08)");
                    break;

                case 0x09: //Coprocessor segment overrun
                    printf("[ERROR] Coprocessor segment overrun (int 0x09)");
                    break;

                case 0x0A: //Invalid task state segment
                    printf("[ERROR] Invalid TSS (int 0x0A)");
                    break;

                case 0x0B: //Segment not present
                    printf("[ERROR] Segment not present (int 0x0B)");
                    break;

                case 0x0C: //Stack segment fault
                    printf("[ERROR] Stack segment fault (int 0x0C)");
                    break;

                case 0x0D: //General protection fault
                    printf("[ERROR] General protection fault (int 0x0D)");
                    break;

                case 0x0E: //Page fault
                    printf("[ERROR] Page fault (int 0x0E)");
                    break;

                case 0x0F: //Reserved
                    printf("[INFO] Reserved (int 0x0F)");
                    break;

                case 0x10: //x87 FPU floating point error
                    printf("[ERROR] x87 FPU floating point error (int 0x10)");
                    break;

                case 0x11: //Alignment check
                    printf("[INFO] Alignment check (int 0x11)");
                    break;

                case 0x12: //Machine check
                    printf("[INFO] Machine check (int 0x12)");
                    break;

                case 0x13: //SIMD floating point exception
                    printf("[ERROR] SIMD floating point exception (int 0x13)");
                    break;

                case 0x14: //Virtualization exception
                    printf("[ERROR] Virtualization exception (int 0x14)");
                    break;

                case 0x15: //Reserved
                    printf("[INFO] Reserved (int 0x15)");
                    break;

                case 0x16: //Reserved
                    printf("[INFO] Reserved (int 0x16)");
                    break;

                case 0x17: //Reserved
                    printf("[INFO] Reserved (int 0x17)");
                    break;

                case 0x18: //Reserved
                    printf("[INFO] Reserved (int 0x18)");
                    break;

                case 0x19: //Reserved
                    printf("[INFO] Reserved (int 0x19)");
                    break;

                case 0x1A: //Reserved
                    printf("[INFO] Reserved (int 0x1A)");
                    break;

                case 0x1B: //Reserved
                    printf("[INFO] Reserved (int 0x1B)");
                    break;

                case 0x1C: //Reserved
                    printf("[INFO] Reserved (int 0x1C)");
                    break;

                case 0x1D: //Reserved
                    printf("[INFO] Reserved (int 0x1D)");
                    break;

                case 0x1E: //Reserved
                    printf("[INFO] Reserved (int 0x1E)");
                    break;

                case 0x1F: //Reserved
                    printf("[INFO] Reserved (int 0x1F)");
                    break;



                default:
                    printf("UNHANDLED INTERRUPT 0x");
                    printfHex(interrupt);
                    printf(" ");
                    break;
                
            }
          

        }
    }

    //TODO: Cast CPUState and uin3t esp to same

    //Timer interrupt for tasks
    if(interrupt == hardwareInterruptOffset)
    {
        esp = (uint32_t)threadManager->Schedule((CPUState_Thread*)esp);


    }

    if(hardwareInterruptOffset <= interrupt && interrupt < hardwareInterruptOffset+16) //Only if it is hardware (keep in mind that around line: 90, the hardware interrupt was remapped at 0x20) the hardware ranges from 0x20 to 0x30
    {
        //Send Answer to tell PIC the interrupt was received
        programmableInterruptControllerMasterCommandPort.Write(0x20);      //0x20 is the answer the PIC wants for master
        if(0x28 <= interrupt)                                              //Answer the master always, but don't answer the slave unless the slave called the interrupt
            programmableInterruptControllerSlaveCommandPort.Write(0x20);   //0x20 is the answer the PIC wants for slave
    }

    CPUState_Thread cpuStateThread = *((CPUState_Thread*)esp);



    return esp;
}

/**
 * @details This function returns the offset of the hardware interrupt
 * @return The offset of the hardware interrupt
 */
maxOS::common::uint16_t InterruptManager::HardwareInterruptOffset() {
    return hardwareInterruptOffset;
}
