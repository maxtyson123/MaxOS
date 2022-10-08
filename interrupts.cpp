//
// Created by 98max on 7/10/2022.
//

#include "interrupts.h"
#include "types.h"
#include "port.h"

void printf(char* str);

struct InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];



void InterruptManager::SetInterruptDescriptorTableEntry(uint8_t interruptNumber, uint16_t codeSegmentSelectorOffset, void (*handler)(), uint8_t DescriptorPrivilegeLevels, uint8_t DescriptorType){
    const uint8_t IDT_DESC_PRESENT = 0x80;

    interruptDescriptorTable[interruptNumber].handlerAdressLowBits = ((uint32_t)handler)  & 0xFFFF;
    interruptDescriptorTable[interruptNumber].handlerAdressHighBits = ((uint32_t)handler >> 16)  & 0xFFFF;
    interruptDescriptorTable[interruptNumber].gdt_codeSegment = codeSegmentSelectorOffset;
    interruptDescriptorTable[interruptNumber].acess = IDT_DESC_PRESENT | DescriptorType | ((DescriptorPrivilegeLevels&3) << 5); //Combine constant with descriptor type and level. The level is shifted by 5 and only the last 3 bits are needed
    interruptDescriptorTable[interruptNumber].reserved = 0;
}


InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
: picMasterCommand(0x20),
  picMasterData(0x21),
  picSlaveCommand(0x21),
  picSlaveData(0x21)
{
    //Set all the entry's to Ignore so that the ones we don't specify aren't run as there won't be a handler for these and therefore protection error
    uint16_t  CodeSegment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;
    for (int i = 0; i < 250; ++i) {
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    }

    //The reason its eg. 0x20 and 0x21 instead of 0 and 1 is becuase it is offest by 0x20 in interstubshit.s line 3

    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);

    //Send Initialization Control Words
    picMasterCommand.Write(0x11);
    picSlaveCommand.Write(0x11);

    //Because the CPU uses interrupt 1 etc, tell the PIC to add 0x20 to it
    picMasterData.Write(0x20);
    picSlaveData.Write(0x20);

    picMasterData.Write(0x04);      //Tell master that it is master
    picSlaveData.Write(0x04);   //Tell slave that it is slave

    //Tell PICS that they are in 8086 mode
    picMasterData.Write(0x01);
    picSlaveData.Write(0x01);

    //Clear  (I Think)
    picMasterData.Write(0x00);
    picSlaveData.Write(0x00);

    //Tell the processor to use the IDT
    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) - 1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt));
};
InterruptManager::~InterruptManager(){

};

void InterruptManager::Activate() {
    asm("sti"); //sti = start interrupts
}

uint32_t InterruptManager::HandleInterrupt(uint8_t interruptNumber, uint32_t esp){
    printf("Interrupt");
    return esp; //This is here for task switching (LATER)
};