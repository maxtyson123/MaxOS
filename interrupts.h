//
// Created by 98max on 7/10/2022.
//

#ifndef MAX_OS_INTERRUPTS_H
#define MAX_OS_INTERRUPTS_H
#include "types.h"
#include "port.h"
#include "gdt.h"

class InterruptManager{
    protected:
        struct GateDescriptor{
            uint16_t handlerAdressLowBits;
            uint16_t gdt_codeSegment;   //Offest
            uint8_t reserved;
            uint8_t acess;
            uint16_t handlerAdressHighBits;
        }__attribute__((packed)); //Prevent Compiler from moving objects as it needs to be byte perfect bc OS stuff

        static GateDescriptor interruptDescriptorTable[256];

        struct InterruptDescriptorTablePointer{
            uint16_t size;
            uint8_t base; //Adress of the table

        }__attribute__((packed)); //Prevent Compiler from moving objects as it needs to be byte perfect bc OS stuff

        static void SetInterruptDescriptorTableEntry(uint8_t interruptNumber, uint16_t codeSegmentSelectorOffset, void (*handler)(), uint8_t DescriptorPrivilegeLevels, uint8_t DescriptorType);

        Port8BitSlow picMasterCommand;
        Port8BitSlow picMasterData;
        Port8BitSlow picSlaveCommand;
        Port8BitSlow picSlaveData;

    public:
        InterruptManager(GlobalDescriptorTable* gdt);
        ~InterruptManager();

        void Activate();

        static uint32_t HandleInterrupt(uint8_t interruptNumber, uint32_t esp);
        static void IgnoreInterruptRequest();
        static void HandleInterruptRequest0x00();   //Timer
        static void HandleInterruptRequest0x01();   //Keyboard

};

#endif //MAX_OS_INTERRUPTS_H
