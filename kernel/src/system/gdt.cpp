//
// Created by 98max on 10/3/2022.
//

#include <system/gdt.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::system;

GlobalDescriptorTable::GlobalDescriptorTable()
{

    Logger::INFO() << "Setting up Global Descriptor Table\n";

    // Null descriptor
    table[0] = 0;

    // Kernel Code Segment descriptor
    uint64_t kernel_cs = 0;
    kernel_cs |= (uint64_t)DescriptorFlags::Write;
    kernel_cs |= (uint64_t)DescriptorFlags::Execute;
    kernel_cs |= (uint64_t)DescriptorFlags::CodeOrDataSegment;
    kernel_cs |= (uint64_t)DescriptorFlags::Present;
    kernel_cs |= (uint64_t)DescriptorFlags::LongMode;
    table[1] = kernel_cs;

    // Kernel Data Segment descriptor
    uint64_t kernel_ds = 0;
    kernel_ds |= (uint64_t)DescriptorFlags::Write;
    kernel_ds |= (uint64_t)DescriptorFlags::CodeOrDataSegment;
    kernel_ds |= (uint64_t)DescriptorFlags::Present;
    table[2] = kernel_ds;

    // User code segment descriptor (Change the privilege level to 3)
    uint64_t user_cs = 0;
    user_cs |= (uint64_t)DescriptorFlags::Write;
    user_cs |= (uint64_t)DescriptorFlags::Execute;
    user_cs |= (uint64_t)DescriptorFlags::CodeOrDataSegment;
    user_cs |= (uint64_t)DescriptorFlags::Present;
    user_cs |= (uint64_t)DescriptorFlags::LongMode;
    user_cs |= (3ULL << 45);
    table[3] = user_cs;

    // User data segment descriptor (Change the privilege level to 3)
    uint64_t user_ds = 0;
    user_ds |= (uint64_t)DescriptorFlags::Write;
    user_ds |= (uint64_t)DescriptorFlags::CodeOrDataSegment;
    user_ds |= (uint64_t)DescriptorFlags::Present;
    user_ds |= (3ULL << 45);
    table[4] = user_ds;

    // Reserve space for the TSS
    table[5] = 0;
    table[6] = 0;

    Logger::DEBUG() << "Created GDT entries\n";

    // Load the GDT
    gdtr_t gdtr = {
       .size    = sizeof(table) - 1,
       .address = (uint64_t)table,
    };
    asm volatile("lgdt %0" : : "m"(gdtr));
    Logger::DEBUG() << "Loaded GDT of limit 0x" << (uint64_t)gdtr.size << " and address 0x" << (uint64_t)gdtr.address << "\n";

    // Reload the segment registers
    asm volatile("mov %0, %%ds" : : "r"(0x10));
    asm volatile("mov %0, %%es" : : "r"(0x10));
    asm volatile("mov %0, %%fs" : : "r"(0x10));
    asm volatile("mov %0, %%gs" : : "r"(0x10));
    asm volatile("mov %0, %%ss" : : "r"(0x10));
    Logger::DEBUG() << "Reloaded segment registers\n";
}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
}