//
// Created by 98max on 10/3/2022.
//

#include <system/gdt.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::system;

GlobalDescriptorTable::GlobalDescriptorTable()
{

   // Null descriptor
    m_gdt[0] = 0;

    // Kernel code segment descriptor
    uint64_t kernel_code_segment_descriptor = 0;
    kernel_code_segment_descriptor |= 0b1011 << 8; // Type of selector
    kernel_code_segment_descriptor |= 1 << 12;     // Not a system descriptor
    kernel_code_segment_descriptor |= 0 << 13;     // Privilege level 0
    kernel_code_segment_descriptor |= 1 << 15;     // Present
    kernel_code_segment_descriptor |= 1 << 21;     // Long mode
    m_gdt[1] = kernel_code_segment_descriptor << 32;

    // Kernel data segment descriptor
    uint64_t kernel_data_segment_descriptor = 0;
    kernel_data_segment_descriptor |= 0b0011 << 8; // Type of selector
    kernel_data_segment_descriptor |= 1 << 12;     // Not a system descriptor
    kernel_data_segment_descriptor |= 0 << 13;     // Privilege level 0
    kernel_data_segment_descriptor |= 1 << 15;     // Present
    kernel_data_segment_descriptor |= 1 << 21;     // Long mode
    m_gdt[2] = kernel_data_segment_descriptor << 32;

    // User code segment descriptor (Change the privilege level to 3)
    uint64_t user_code_segment_descriptor = kernel_code_segment_descriptor | (3 << 13);
    m_gdt[3] = user_code_segment_descriptor;

    // User data segment descriptor (Change the privilege level to 3)
    uint64_t user_data_segment_descriptor = kernel_data_segment_descriptor | (3 << 13);
    m_gdt[4] = user_data_segment_descriptor;

    _kprintf("Created GDT entries\n");


    // TODO: This is done in assembly but would be nice to do here


    _kprintf("Reloaded segment registers\n");

}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
}