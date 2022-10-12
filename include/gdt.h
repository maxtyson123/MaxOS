//
// Created by 98max on 10/3/2022.
//

#ifndef MAX_OS_GDT_H
#define MAX_OS_GDT_H

#include <common/types.h>
namespace maxos {

    class GlobalDescriptorTable {
    public:
        class SegmentDescriptor {
        private:
            maxos::common::uint16_t limit_lo;      //Low Bytes of the pointer       (Least Significant)
            maxos::common::uint16_t base_lo;       //Low Bytes of the pointer        (Least Significant)
            maxos::common::uint8_t base_hi;        //One byte extension for the pointer
            maxos::common::uint8_t type;           //Excess Bytes
            maxos::common::uint8_t flags_limit_hi; //High Bytes of the pointer        (Most Significant)
            maxos::common::uint8_t base_vhi;       //High Bytes of the pointer        (Most Significant)
        public:
            SegmentDescriptor(maxos::common::uint32_t base, maxos::common::uint32_t limit, maxos::common::uint8_t type);

            //Return Pointer and the limit
            maxos::common::uint32_t Base();

            maxos::common::uint32_t Limit();

        } __attribute__((packed)); //Tell GCC not to change any of the alignment in the structure. The packed attribute specifies that a variable or structure field should have the smallest possible alignment, one byte for a variable, and one bit for a field, unless you specify a larger value with the aligned attribute.

    private:
        SegmentDescriptor nullSegmentSelector;
        SegmentDescriptor unusedSegmentSelector;
        SegmentDescriptor codeSegmentSelector;
        SegmentDescriptor dataSegmentSelector;

    public:

        GlobalDescriptorTable();

        ~GlobalDescriptorTable();

        maxos::common::uint16_t CodeSegmentSelector();

        maxos::common::uint16_t DataSegmentSelector();
    };
}

#endif //MAX_OS_GDT_H
