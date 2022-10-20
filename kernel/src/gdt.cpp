//
// Created by 98max on 10/3/2022.
//

#include <gdt.h>
using namespace maxOS;
using namespace maxOS::common;

GlobalDescriptorTable::GlobalDescriptorTable()
        : nullSegmentSelector(0, 0, 0),                     //Ignored
          unusedSegmentSelector(0, 0, 0),                   //Ignored
          codeSegmentSelector(0, 64*1024*1024, 0x9A),       //0x9A Access for code
          dataSegmentSelector(0, 64*1024*1024, 0x92)        //0x92 Access flag for data
{
    //Tell processor to use this table   (8 bytes)
    uint32_t gdt_t[2];
    gdt_t[0] = sizeof(GlobalDescriptorTable) << 16;             //First and Second byte: The high  bytes of the segment integer
    gdt_t[1] = (uint32_t)this;                                  //Last Four bytes: Tell processor the address of table

    asm volatile("lgdt (%0)": :"p" (((uint8_t *) gdt_t)+2));    //Pass it as an unsigned 8Bit int to assembly (p means push adress)

}


GlobalDescriptorTable::~GlobalDescriptorTable()
{
}


//Off sets
uint16_t GlobalDescriptorTable::DataSegmentSelector()
{
    return (uint8_t*)&dataSegmentSelector - (uint8_t*)this;
}

uint16_t GlobalDescriptorTable::CodeSegmentSelector()
{
    return (uint8_t*)&codeSegmentSelector - (uint8_t*)this;
}

//Setup GDT for memory
GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type)
{
    uint8_t* target = (uint8_t*)this;       //segment entry in GDT.

    if (limit <= 65536)
    {
        // 16-bit address space
        target[6] = 0x40;
    }
    else
    {
        // 32-bit address space
        // Now we have to squeeze the (32-bit) limit into 2.5 registers (20-bit).
        // This is done by discarding the 12 least significant bits, but this
        // is only legal, if they are all ==1, so they are implicitly still there

        // so if the last bits aren't all 1, we have to set them to 1, but this
        // would increase the limit (cannot do that, because we might go beyond
        // the physical limit or get overlap with other segments) so we have to
        // compensate this by decreasing a higher bit (and might have up to
        // 4095 wasted bytes behind the used memory)


        if((limit & 0xFFF) != 0xFFF)
            limit = (limit >> 12)-1;
        else
            limit = limit >> 12;

        target[6] = 0xC0;
    }

    // Encode the limit
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] |= (limit >> 16) & 0xF;

    // Encode the base
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;

    // Type / Access Rights
    target[5] = type;
}

//To look up the pointer
uint32_t GlobalDescriptorTable::SegmentDescriptor::Base()
{
    uint8_t* target = (uint8_t*)this;

    uint32_t result = target[7];
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];

    return result;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit()
{
    uint8_t* target = (uint8_t*)this;

    uint32_t result = target[6] & 0xF;
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];

    if((target[6] & 0xC0) == 0xC0)
        result = (result << 12) | 0xFFF;

    return result;
}
