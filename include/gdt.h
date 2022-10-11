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
            maxos::common::uint16_t limit_lo;      //Low Bytes of the pointer
            maxos::common::uint16_t base_lo;       //Low Bytes of the pointer
            maxos::common::uint8_t base_hi;        //One byte extension for the pointer
            maxos::common::uint8_t type;           //Excess Bytes
            maxos::common::uint8_t flags_limit_hi;
            maxos::common::uint8_t base_vhi;
        public:
            SegmentDescriptor(maxos::common::uint32_t base, maxos::common::uint32_t limit, maxos::common::uint8_t type);

            //Return Pointer and the limit
            maxos::common::uint32_t Base();

            maxos::common::uint32_t Limit();

        } __attribute__((packed)); //Prevent Compiler from moving objects as it needs to be byte perfect bc OS stuff

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
