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
         * @static DescriptorFlags
         * @brief Flags for the GDT entries
         */
         enum class DescriptorFlags : uint64_t {
             Write               = (1ULL << 41),
             Execute             = (1ULL << 43),
             CodeOrDataSegment   = (1ULL << 44),
             GrowDown            = (1ULL << 45),
             ConformFromLower    = (1ULL << 46),
             Present             = (1ULL << 47),
             LongMode            = (1ULL << 53),
         };


        /**
         * @struct GlobalDescriptorTableRegister
         * @brief How the CPU stores the GDT
         */
        typedef struct GlobalDescriptorTableRegister {
            uint16_t size;
            uint64_t address;
        } __attribute__((packed)) gdtr_t;

        /**
         * @class GlobalDescriptorTable
         * @brief Sets up the GDT in the CPU
         */
        class GlobalDescriptorTable {

          public:
              GlobalDescriptorTable();
              ~GlobalDescriptorTable();

              uint64_t table[7];

			  gdtr_t gdtr = {};
          };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
