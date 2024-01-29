//
// Created by 98max on 18/01/2024.
//

#ifndef MAXOS_HARDWARECOMMUNICATION_APIC_H
#define MAXOS_HARDWARECOMMUNICATION_APIC_H

#include <hardwarecommunication/port.h>
#include <system/cpu.h>
#include <hardwarecommunication/acpi.h>

namespace MaxOS {
    namespace hardwarecommunication {


      class  LocalAPIC {

        protected:
          uint64_t m_apic_base;
          uint32_t m_id;
          bool m_x2apic;

          uint32_t read(uint32_t reg);
          void write(uint32_t reg, uint32_t value);

        public:
            LocalAPIC();
            ~LocalAPIC();

            void init();

            uint32_t id();

        };

        struct MADT {
          ACPISDTHeader header;
          uint32_t local_apic_address;
          uint32_t flags;
        } __attribute__((packed));

        struct MADT_Item {
          uint8_t type;
          uint8_t length;
        } __attribute__((packed));

        struct MADT_IOAPIC {
          uint8_t io_apic_id;
          uint8_t reserved;
          uint32_t io_apic_address;
          uint32_t global_system_interrupt_base;
        } __attribute__((packed));

        union RedirectionEntry {
          struct {
            uint64_t vector : 8;
            uint64_t delivery_mode : 3;
            uint64_t destination_mode : 1;
            uint64_t delivery_status : 1;
            uint64_t pin_polarity : 1;
            uint64_t remote_irr : 1;
            uint64_t trigger_mode : 1;
            uint64_t mask : 1;
            uint64_t reserved : 39;
            uint64_t destination : 8;
          } __attribute__((packed));

          uint64_t raw;
        };

        struct Override {
          uint8_t bus;
          uint8_t source;
          uint32_t global_system_interrupt;
          uint16_t flags;
        } __attribute__((packed));

        class IOAPIC {
          private:
            AdvancedConfigurationAndPowerInterface* m_acpi;
            MADT* m_madt;
            uint32_t m_address;
            uint32_t m_version;
            uint8_t m_max_redirect_entry;

            uint32_t m_override_array_size;
            Override m_override_array[0x10];

            MADT_Item* get_madt_item(uint8_t type, uint8_t index);

            uint32_t read(uint32_t reg);
            void write(uint32_t reg, uint32_t value);

            void read_redirect(uint8_t index, RedirectionEntry* entry);
            void write_redirect(uint8_t index, RedirectionEntry* entry);

          public:
              IOAPIC(AdvancedConfigurationAndPowerInterface* acpi);
              ~IOAPIC();

              void init();
        };

      class AdvancedProgrammableInterruptController {

        protected:
            LocalAPIC m_local_apic;
            IOAPIC m_io_apic;

            Port8BitSlow m_pic_master_command_port;
            Port8BitSlow m_pic_master_data_port;
            Port8BitSlow m_pic_slave_command_port;
            Port8BitSlow m_pic_slave_data_port;

            void disable_pic();

        public:
            AdvancedProgrammableInterruptController(AdvancedConfigurationAndPowerInterface* acpi);
            ~AdvancedProgrammableInterruptController();
        };

    }
}

#endif // MAXOS_HARDWARECOMMUNICATION_APIC_H
