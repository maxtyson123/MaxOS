//
// Created by 98max on 7/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
#define MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <system/gdt.h>
#include <system/multithreading.h>
#include <common/inputStream.h>
#include <common/outputStream.h>


namespace maxOS {
    namespace hardwarecommunication {

        class InterruptManager;

        class InterruptHandler {
            protected:
                common::uint8_t interrupNumber;
                InterruptManager *interruptManager;

                InterruptHandler(common::uint8_t interrupNumber, InterruptManager *interruptManager);

                ~InterruptHandler();

            public:
                virtual common::uint32_t HandleInterrupt(common::uint32_t esp);

        };

        class InterruptManager : public common::InputStream {
            friend class InterruptHandler;

            protected:

                static InterruptManager *ActiveInterruptManager;
                InterruptHandler *interruptHandlers[256];
                ThreadManager* threadManager;

                struct GateDescriptor {
                    common::uint16_t handlerAddressLowBits;
                    common::uint16_t gdt_codeSegmentSelector;
                    common::uint8_t reserved;
                    common::uint8_t access;
                    common::uint16_t handlerAddressHighBits;
                } __attribute__((packed));

                static GateDescriptor interruptDescriptorTable[256];

                struct InterruptDescriptorTablePointer {
                    common::uint16_t size;
                    common::uint32_t base;
                } __attribute__((packed));

                common::uint16_t hardwareInterruptOffset;
                common::OutputStream* errorMessages;

                static void SetInterruptDescriptorTableEntry(common::uint8_t interrupt,
                                                             common::uint16_t codeSegmentSelectorOffset,
                                                             void (*handler)(),
                                                             common::uint8_t DescriptorPrivilegeLevel,
                                                             common::uint8_t DescriptorType);

                static void InterruptIgnore();

                //Various Interrupts
                static void HandleInterruptRequest0x00();
                static void HandleInterruptRequest0x01();
                static void HandleInterruptRequest0x02();
                static void HandleInterruptRequest0x03();
                static void HandleInterruptRequest0x04();
                static void HandleInterruptRequest0x05();
                static void HandleInterruptRequest0x06();
                static void HandleInterruptRequest0x07();
                static void HandleInterruptRequest0x08();
                static void HandleInterruptRequest0x09();
                static void HandleInterruptRequest0x0A();
                static void HandleInterruptRequest0x0B();
                static void HandleInterruptRequest0x0C();
                static void HandleInterruptRequest0x0D();
                static void HandleInterruptRequest0x0E();
                static void HandleInterruptRequest0x80();
                static void HandleInterruptRequest0x0F();
                static void HandleInterruptRequest0x31();

                static void HandleException0x00();
                static void HandleException0x01();
                static void HandleException0x02();
                static void HandleException0x03();
                static void HandleException0x04();
                static void HandleException0x05();
                static void HandleException0x06();
                static void HandleException0x07();
                static void HandleException0x08();
                static void HandleException0x09();
                static void HandleException0x0A();
                static void HandleException0x0B();
                static void HandleException0x0C();
                static void HandleException0x0D();
                static void HandleException0x0E();
                static void HandleException0x0F();
                static void HandleException0x10();
                static void HandleException0x11();
                static void HandleException0x12();
                static void HandleException0x13();
                static void HandleException0x14();
                static void HandleException0x15();
                static void HandleException0x16();
                static void HandleException0x17();
                static void HandleException0x18();
                static void HandleException0x19();
                static void HandleException0x1A();
                static void HandleException0x1B();
                static void HandleException0x1C();
                static void HandleException0x1D();
                static void HandleException0x1E();
                static void HandleException0x1F();


                static common::uint32_t HandleInterrupt(common::uint8_t interrupt, common::uint32_t esp);

                common::uint32_t DoHandleInterrupt(common::uint8_t interrupt, common::uint32_t esp);

                //PIC Cominunication
                Port8BitSlow programmableInterruptControllerMasterCommandPort;
                Port8BitSlow programmableInterruptControllerMasterDataPort;
                Port8BitSlow programmableInterruptControllerSlaveCommandPort;
                Port8BitSlow programmableInterruptControllerSlaveDataPort;


            public:
                InterruptManager(common::uint16_t hardwareInterruptOffset, system::GlobalDescriptorTable *globalDescriptorTable, ThreadManager* threadManage, common::OutputStream* handler);

                ~InterruptManager();

                common::uint16_t HardwareInterruptOffset();

                void Activate();

                void Deactivate();

        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
