//
// Created by 98max on 7/10/2022.
//

#include <hardwarecommunication/interrupts.h>
using namespace maxos::common;
using namespace maxos::hardwarecommunication;


void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration

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


InterruptManager::InterruptManager(uint16_t hardwareInterruptOffset, GlobalDescriptorTable* globalDescriptorTable)
        : programmableInterruptControllerMasterCommandPort(0x20),
          programmableInterruptControllerMasterDataPort(0x21),
          programmableInterruptControllerSlaveCommandPort(0xA0),
          programmableInterruptControllerSlaveDataPort(0xA1)
{
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
    SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);

    //Set up the hardware interrupts (offest by 0x20) //Ranges 0x20 - 0x30
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);  //0x20 - Default PIC interval
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x01, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);  //0x21 - Keyboard
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x02, CodeSegment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x03, CodeSegment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x04, CodeSegment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x05, CodeSegment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x06, CodeSegment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x07, CodeSegment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x08, CodeSegment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x09, CodeSegment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0A, CodeSegment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0B, CodeSegment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);  //0x0C - Mouse
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0D, CodeSegment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0E, CodeSegment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0F, CodeSegment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);

    //Send Initialization Control Words
    programmableInterruptControllerMasterCommandPort.Write(0x11);
    programmableInterruptControllerSlaveCommandPort.Write(0x11);

    //Because the CPU uses interrupt 1 etc, tell the PIC to add 0x20 to it (this is the remapping of hardware)
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

void InterruptManager::Activate() {

    if(ActiveInterruptManager != 0){                //There shouldnt be another interrupt manager, but for saftey delete anyother ones. This is becuase the processor only has 1 IDT
        ActiveInterruptManager->Deactivate();
    }

    ActiveInterruptManager = this;
    asm("sti"); //sti = start interrupts

}

void InterruptManager::Deactivate()
{
    if(ActiveInterruptManager == this){         //Only if the active InterruptManger
        ActiveInterruptManager = 0;
        asm("cli");                             //cli = clear interrupts
    }


}

uint32_t InterruptManager::HandleInterrupt(uint8_t interrupt, uint32_t esp)
{

    if(ActiveInterruptManager != 0){
        return ActiveInterruptManager->DoHandleInterrupt(interrupt, esp);       //Handle the interrupt in OOP mode instead of Static
    }

    return esp;
}


uint32_t InterruptManager::DoHandleInterrupt(uint8_t interrupt, uint32_t esp)
{
    if(handlers[interrupt]!= 0){                                //If it has a handler for it
        esp = handlers[interrupt]->HandleInterrupt(esp);        //Run the handler
    }else{
        if(interrupt != 0x20){   //If not the timer interrupt
            printf("UNHANDLED INTERRUPT 0x");
            printfHex(interrupt);
        }
    }

    if(0x20 <= interrupt && interrupt < 0x30) //Only if it is hardware (keep in mind that around line: 90, the hardware interrupt was remapped at 0x20) the hardware ranges from 0x20 to 0x30
    {
        //Send Answer to tell PIC the interrupt was received
        programmableInterruptControllerMasterCommandPort.Write(0x20);      //0x20 is the answer the PIC wants for master
        if(0x28 <= interrupt)                                              //Answer the master always, but don't answer the slave unless the slave called the interrupt
            programmableInterruptControllerSlaveCommandPort.Write(0x20);   //0x20 is the answer the PIC wants for slave
    }
    return esp;
}