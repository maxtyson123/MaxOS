//
// Created by 98max on 7/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
#define MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H

#include <stdint.h>
#include <hardwarecommunication/port.h>
#include <system/gdt.h>
#include <processes/scheduler.h>
#include <common/inputStream.h>
#include <common/outputStream.h>
#include <system/cpu.h>
#include <hardwarecommunication/apic.h>


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
         * @struct IDTR
         * @brief A struct that holds the IDT register
         */
        struct IDTR {
            uint16_t limit;
            uint64_t base;
        } __attribute__((packed));

        struct InterruptDescriptor{
          uint16_t address_low_bits;
          uint16_t segment_selector;
          uint8_t ist;
          uint8_t flags;
          uint16_t address_mid_bits;
          uint32_t address_high_bits;
          uint32_t reserved;
        } __attribute__((packed));

        /**
         * @class InterruptManager
         * @brief Handles all interrupts and passes them to the correct handler
         */
        class InterruptManager {
            friend class InterruptHandler;

            private:
              LocalAPIC* m_local_apic;

              static void page_fault(system::cpu_status_t* status);
              static void general_protection_fault(system::cpu_status_t* status);

            protected:

                static InterruptManager* s_active_interrupt_manager;
                static common::OutputStream* s_error_messages;
                const static uint16_t s_hardware_interrupt_offset {0x20};
                InterruptHandler* m_interrupt_handlers[256];

                static InterruptDescriptor s_interrupt_descriptor_table[256];

                static void set_interrupt_descriptor_table_entry(uint8_t interrupt, void (*handler)(), uint8_t descriptor_privilege_level);

                static void InterruptIgnore();

                // Error Codes
                // TODO

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
                static void HandleInterruptRequest0x60();

                // Exceptions
                static void HandleException0x00();
                static void HandleException0x01();
                static void HandleException0x02();
                static void HandleException0x03();
                static void HandleException0x04();
                static void HandleException0x05();
                static void HandleException0x06();
                static void HandleException0x07();
                static void HandleInterruptError0x08();
                static void HandleException0x09();
                static void HandleInterruptError0x0A();
                static void HandleInterruptError0x0B();
                static void HandleInterruptError0x0C();
                static void HandleInterruptError0x0D();
                static void HandleInterruptError0x0E();
                static void HandleException0x0F();
                static void HandleException0x10();
                static void HandleInterruptError0x11();
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

                static system::cpu_status_t* HandleInterrupt(system::cpu_status_t* status);

                system::cpu_status_t* handle_interrupt_request(system::cpu_status_t*);

            public:
                InterruptManager();
                ~InterruptManager();

                uint16_t hardware_interrupt_offset();

                void set_interrupt_handler(uint8_t interrupt, InterruptHandler *handler);
                void remove_interrupt_handler(uint8_t interrupt);

                void set_apic(LocalAPIC* apic);

                void activate();
                void deactivate();

        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
