//
// Created by 98max on 10/3/2022.
//

#ifndef MAX_OS_SYSTEM_GDT_H
#define MAX_OS_SYSTEM_GDT_H

#include <stdint.h>
#include <system/multiboot.h>

namespace maxOS {
    namespace system {
        class GlobalDescriptorTable {
        public:
            class SegmentDescriptor {
            private:
                uint16_t limit_lo;      //Low Bytes of the pointer       (Least Significant)
                uint16_t base_lo;       //Low Bytes of the pointer        (Least Significant)
                uint8_t base_hi;        //One byte extension for the pointer
                uint8_t type;           //Excess Bytes
                uint8_t flags_limit_hi; //High Bytes of the pointer        (Most Significant)
                uint8_t base_vhi;       //High Bytes of the pointer        (Most Significant)
            public:
                SegmentDescriptor(uint32_t base, uint32_t limit,
                                  uint8_t type);

                //Return Pointer and the limit
                uint32_t Base();

                uint32_t Limit();

            } __attribute__((packed)); //Tell GCC not to change any of the alignment in the structure. The packed attribute specifies that a variable or structure field should have the smallest possible alignment, one byte for a variable, and one bit for a field, unless you specify a larger value with the aligned attribute.

        private:
            SegmentDescriptor nullSegmentSelector;
            SegmentDescriptor unusedSegmentSelector;
            SegmentDescriptor codeSegmentSelector;
            SegmentDescriptor dataSegmentSelector;
            SegmentDescriptor taskStateSegmentSelector;

        public:

            GlobalDescriptorTable(const multiboot_info& multibootHeader);
            ~GlobalDescriptorTable();

            uint16_t CodeSegmentSelector();
            uint16_t DataSegmentSelector();
            uint16_t TaskStateSegmentSelector();
        };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
