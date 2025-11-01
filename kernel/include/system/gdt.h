/**
 * @file gdt.h
 * @brief Defines a Global Descriptor Table (GDT) for setting up memory segments in protected mode
 *
 * @date 3rd October 2022
 * @author Max Tyson
 */

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
         * @struct GDTRegister
         * @brief How the CPU stores the GDT
         */
        typedef struct GDTRegister {
            uint16_t size;      ///< The size of the GDT
            uint64_t address;   ///< The address of the GDT
        } __attribute__((packed)) gdtr_t;

        /**
         * @class GlobalDescriptorTable
         * @brief Sets up the GDT in the CPU
         */
        class GlobalDescriptorTable {

          public:
              GlobalDescriptorTable();
              ~GlobalDescriptorTable();

              uint64_t table[7];    ///< The GDT entries

			  gdtr_t gdtr = {};     ///< The GDTR structure
			  void load();
          };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
