//
// Created by 98max on 10/3/2022.
//

#ifndef MAX_OS_SYSTEM_GDT_H
#define MAX_OS_SYSTEM_GDT_H

#include <common/types.h>
namespace maxOS {
    namespace system {
        class GlobalDescriptorTable {
        public:
            class SegmentDescriptor {
            private:
                maxOS::common::uint16_t limit_lo;      //Low Bytes of the pointer       (Least Significant)
                maxOS::common::uint16_t base_lo;       //Low Bytes of the pointer        (Least Significant)
                maxOS::common::uint8_t base_hi;        //One byte extension for the pointer
                maxOS::common::uint8_t type;           //Excess Bytes
                maxOS::common::uint8_t flags_limit_hi; //High Bytes of the pointer        (Most Significant)
                maxOS::common::uint8_t base_vhi;       //High Bytes of the pointer        (Most Significant)
            public:
                SegmentDescriptor(maxOS::common::uint32_t base, maxOS::common::uint32_t limit,
                                  maxOS::common::uint8_t type);

                //Return Pointer and the limit
                maxOS::common::uint32_t Base();

                maxOS::common::uint32_t Limit();

            } __attribute__((packed)); //Tell GCC not to change any of the alignment in the structure. The packed attribute specifies that a variable or structure field should have the smallest possible alignment, one byte for a variable, and one bit for a field, unless you specify a larger value with the aligned attribute.

        private:
            SegmentDescriptor nullSegmentSelector;
            SegmentDescriptor unusedSegmentSelector;
            SegmentDescriptor codeSegmentSelector;
            SegmentDescriptor dataSegmentSelector;

        public:

            GlobalDescriptorTable();

            ~GlobalDescriptorTable();

            maxOS::common::uint16_t CodeSegmentSelector();

            maxOS::common::uint16_t DataSegmentSelector();
        };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
