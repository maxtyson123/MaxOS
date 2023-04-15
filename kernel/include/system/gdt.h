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
                common::uint16_t limit_lo;      //Low Bytes of the pointer       (Least Significant)
                common::uint16_t base_lo;       //Low Bytes of the pointer        (Least Significant)
                common::uint8_t base_hi;        //One byte extension for the pointer
                common::uint8_t type;           //Excess Bytes
                common::uint8_t flags_limit_hi; //High Bytes of the pointer        (Most Significant)
                common::uint8_t base_vhi;       //High Bytes of the pointer        (Most Significant)
            public:
                SegmentDescriptor(common::uint32_t base, common::uint32_t limit,
                                  common::uint8_t type);

                //Return Pointer and the limit
                common::uint32_t Base();

                common::uint32_t Limit();

            } __attribute__((packed)); //Tell GCC not to change any of the alignment in the structure. The packed attribute specifies that a variable or structure field should have the smallest possible alignment, one byte for a variable, and one bit for a field, unless you specify a larger value with the aligned attribute.

        private:
            SegmentDescriptor nullSegmentSelector;
            SegmentDescriptor unusedSegmentSelector;
            SegmentDescriptor codeSegmentSelector;
            SegmentDescriptor dataSegmentSelector;

        public:

            GlobalDescriptorTable();

            ~GlobalDescriptorTable();

            common::uint16_t CodeSegmentSelector();

            common::uint16_t DataSegmentSelector();
        };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
