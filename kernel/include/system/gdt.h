//
// Created by 98max on 10/3/2022.
//

#ifndef MAX_OS_SYSTEM_GDT_H
#define MAX_OS_SYSTEM_GDT_H

#include <stdint.h>
#include <system/multiboot.h>

namespace MaxOS {
    namespace system {


        /**
         * @struct GDTR
         * @brief A struct used to store information for the GDT Register
         */
        struct GDTR {
            uint16_t limit;
            uint64_t address;
        } __attribute__((packed));

        /**
         * @class GlobalDescriptorTable
         * @brief Sets up the GDT in the CPU
         */
        class GlobalDescriptorTable {

          uint64_t m_gdt[4];

          public:
              GlobalDescriptorTable();
              ~GlobalDescriptorTable();
          };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
