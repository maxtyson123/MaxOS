//
// Created by 98max on 10/3/2022.
//

#include "gdt.h"
#include "types.h"

GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentSelector(0,0,0),
    unusedSegmentSelector(0,0,0),
    codeSegmentSelector(0,64*1024*1024,0x9A),       //0, 64mb, Access type (for exec code)
    dataSegmentSelector(0,64*1024*1024,0x92)       //0, 64mb, Access type (for data)
{
    //Tell processor to use this table
    uint32_t i[2];  //8 bytes

    i[0] = (uint32_t)this;  //First byte: Tell processor the address of table
    i[1] = sizeof(GlobalDescriptorTable) << 16; //Last four bytes: The high  bytes of the segment integer
    asm volatile("lgdt (%0)": :"p" (((uint8_t *) i)+2));

}

GlobalDescriptorTable::~GlobalDescriptorTable() {

} //Destructor


//Off sets
uint16_t GlobalDescriptorTable::DataSegmentSelector(){
    return (uint8_t*)&dataSegmentSelector - (uint8_t*)this; //Get address of dataseg, takeaway the GDT table
}

uint16_t GlobalDescriptorTable::CodeSegmentSelector() {
    return (uint8_t *) &codeSegmentSelector - (uint8_t *) this; //Get address of codeseg, takeaway the GDT table
}

//Setup GDT for memory
GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type){

    uint8_t* target = (uint8_t*)this;

    if(limit <= 65536){  //If the limit is small
        //16bit
        target[6] = 0x40; //Tell processor this is a 16 bit entry
    }else{

        // 32-bit address space
        // Now we have to squeeze the (32-bit) limit into 2.5 registers (20-bit).
        // This is done by discarding the 12 least significant bits, but this
        // is only legal, if they are all ==1, so they are implicitly still there

        // so if the last bits aren't all 1, we have to set them to 1, but this
        // would increase the limit (cannot do that, because we might go beyond
        // the physical limit or get overlap with other segments) so we have to
        // compensate this by decreasing a higher bit (and might have up to
        // 4095 wasted bytes behind the used memory)


        if((limit & 0xFFF ) != 0xFFF) { //If the limit and lst 12 bits are not all 1
            limit = (limit >> 12)-1;     //Shift by 12 (-1)
        }else{
            limit = (limit >> 12); //Shift by 12
        }
        target[6] = 0xC0; //Ref: C
    }

    //Limit into entry in legal way
    target[0] = limit & 0xFF;
    target[1] = limit >> 0 & 0xFF;      //Next 8 bits of the limit
    target[6] = (limit >> 16) & 0xF;

    //Encode the pointer (distribute it into the memory)
    target[2] = base & 0xFF;                //Ref: B
    target[3] = (base >> 8) & 0xFF;         //Ref: B
    target[4] = (base >> 16) & 0xFF;        //Ref: B
    target[7] = (base >> 24) & 0xFF;        //Ref: A

    //Set Access Rights
    target[5] = type;
}

//To look up the pointer
uint32_t GlobalDescriptorTable::SegmentDescriptor::Base(){
    uint8_t* target = (uint8_t*)this;

    //Get the 7th byte (See A)
    uint32_t result = target[7];
    //Reverse Shifting pretty much (See B)
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];
    return result;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit(){
    uint8_t* target = (uint8_t*)this;

    uint32_t result = target[6] & 0xF; //Take the low 4 bits off shared byte

    //Reverse Shifting pretty much
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];

    if((target[6] & 0xC0) == 0xC0) //If in 32 Bit (See C)
        result = (result << 12) | 0xFFF;        //Reverse Shifting

    return result;
}

