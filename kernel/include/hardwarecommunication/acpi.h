//
// Created by 98max on 18/01/2024.
//

#ifndef MAXOS_HARDWARECOMMUNICATION_ACPI_H
#define MAXOS_HARDWARECOMMUNICATION_ACPI_H

#include <stdint.h>
#include <stddef.h>
#include <system/multiboot.h>
#include <common/string.h>
#include <memory/memorymanagement.h>
#include <memory/physical.h>

namespace MaxOS {
    namespace hardwarecommunication {

      struct RSDPDescriptor {
        char signature[8];
        uint8_t checksum;
        char OEMID[6];
        uint8_t revision;
        uint32_t rsdt_address;
      } __attribute__ ((packed));

      struct RSDPDescriptor2 {
        RSDPDescriptor firstPart;
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
        uint8_t reserved[3];
      } __attribute__ ((packed));

      struct ACPISDTHeader {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char OEM_id[6];
        char OEM_table_id[8];
        uint32_t OEM_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
      } __attribute__ ((packed));

      struct RSDT {
        ACPISDTHeader header;
        uint32_t pointers[];
      };

      struct XSDT {
        ACPISDTHeader header;
        uint64_t pointers[];
      };

      class AdvancedConfigurationAndPowerInterface {
        protected:
          uint8_t m_type;
          ACPISDTHeader* m_header;

          XSDT* m_xsdt;
          RSDT* m_rsdt;

          static bool validate(const char*descriptor, size_t length);

        public:
            explicit AdvancedConfigurationAndPowerInterface(system::Multiboot* multiboot);
            ~AdvancedConfigurationAndPowerInterface();

            ACPISDTHeader* find(const char* signature);
      };

    }
}

#endif // MAXOS_HARDWARECOMMUNICATION_ACPI_H
