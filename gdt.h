//
// Created by 98max on 10/3/2022.
//

#ifndef MAX_OS_GDT_H
#define MAX_OS_GDT_H

#include "types.h"
class GlobalDescriptorTable{
    public:
        class SegmentDescriptor{
            private:
                uint16_t limit_lo;      //Low Bytes of the pointer
                uint16_t base_lo;       //Low Bytes of the pointer
                uint8_t base_hi;        //One byte extension for the pointer
                uint8_t type;           //Excess Bytes
                uint8_t flags_limit_hi;
                uint8_t base_vhi;
        public:
            SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type);
            //Return Pointer and the limit
            uint32_t Base();
            uint32_t Limit();

        } __attribute__((packed)); //Prevent Compiler from moving objects as it needs to be byte perfect bc OS stuff

        private:
            SegmentDescriptor nullSegmentSelector;
            SegmentDescriptor unusedSegmentSelector;
            SegmentDescriptor codeSegmentSelector;
            SegmentDescriptor dataSegmentSelector;

        public:

            GlobalDescriptorTable();
            ~GlobalDescriptorTable();

            uint16_t CodeSegmentSelector();
            uint16_t DataSegmentSelector();
        };

#endif //MAX_OS_GDT_H
