//
// Created by 98max on 7/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
#define MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H

#include <stdint.h>
#include <hardwarecommunication/port.h>
#include <system/gdt.h>
#include <system/multithreading.h>
#include <common/inputStream.h>
#include <common/outputStream.h>


namespace MaxOS {
    namespace hardwarecommunication {

        class InterruptManager;

        /**
         * @class InterruptHandler
         * @brief Handles a certain interrupt number
         */
        class InterruptHandler {
            protected:
                uint8_t m_interrupt_number;
                InterruptManager* m_interrupt_manager;

                InterruptHandler(uint8_t interrupt_number, InterruptManager*interrupt_manager = 0);
                ~InterruptHandler();

            public:
                virtual void handle_interrupt();

        };

        /**
         * @struct GateDescriptor
         * @brief Describes a gate in the Interrupt Descriptor Table
         */
        struct GateDescriptor {
          uint16_t  handler_address_low_bits;
          uint16_t  gdt_code_segment_selector;
          uint8_t   reserved;
          uint8_t   access;
          uint16_t handler_address_high_bits;
        } __attribute__((packed));

        /**
         * @struct InterruptDescriptorTablePointer
         * @brief Describes the Interrupt Descriptor Table
         */
        struct InterruptDescriptorTablePointer {
          uint16_t size;
          uint32_t base;
        } __attribute__((packed));

        /**
         * @class InterruptManager
         * @brief Handles all interrupts and passes them to the correct handler
         */
        class InterruptManager : public common::InputStream {
            friend class InterruptHandler;

            protected:

                static InterruptManager* s_active_interrupt_manager;
                static common::OutputStream* s_error_messages;
                uint16_t m_hardware_interrupt_offset;
                InterruptHandler* m_interrupt_handlers[256];
                system::ThreadManager* m_thread_manager;

                static GateDescriptor s_interrupt_descriptor_table[256];

                static void set_interrupt_descriptor_table_entry(uint8_t interrupt, uint16_t code_segment_selector_offset, void (*handler)(), uint8_t DescriptorPrivilegeLevel, uint8_t descriptor_type);

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


                static uint32_t HandleInterrupt(uint8_t interrupt, uint32_t esp);
                static uint32_t HandleInterruptRequest(uint32_t esp);
                uint32_t handle_interrupt_request(uint8_t interrupt, uint32_t esp);

                //PIC Cominunication
                Port8BitSlow pic_master_command_port;
                Port8BitSlow pic_master_data_port;
                Port8BitSlow pic_slave_command_port;
                Port8BitSlow pic_slave_data_port;


            public:
                InterruptManager(uint16_t hardware_interrupt_offset, system::GlobalDescriptorTable*global_descriptor_table, system::ThreadManager* thread_manager, common::OutputStream* handler);
                ~InterruptManager();

                uint16_t hardware_interrupt_offset();

                void set_interrupt_handler(uint8_t interrupt, InterruptHandler *handler);
                void remove_interrupt_handler(uint8_t interrupt);

                void activate();
                void deactivate();

        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
