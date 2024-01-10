//
// Created by 98max on 10/3/2022.
//

#include <system/gdt.h>
using namespace maxOS;
using namespace maxOS::system;


/**
 * @brief Constructor for Segment Selector
 *
 *
 * @param base base address
 * @param limit limit
 * @param flags Flags
 */
SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type)
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
/**
 * @brief Look up the pointer
 *
 * @return The pointer
 */
uint32_t SegmentDescriptor::base()
{
  uint8_t* target = (uint8_t*)this;

  uint32_t result = target[7];
  result = (result << 8) + target[4];
  result = (result << 8) + target[3];
  result = (result << 8) + target[2];

  return result;
}

/**
 * @brief Look up the limit
 *
 * @return The limit
 */
uint32_t SegmentDescriptor::limit()
{
  uint8_t* target = (uint8_t*)this;

  uint32_t result = target[6] & 0xF;
  result = (result << 8) + target[1];
  result = (result << 8) + target[0];

  if((target[6] & 0xC0) == 0xC0)
    result = (result << 12) | 0xFFF;

  return result;
}

/**
 * @brief Global Descriptor Table
 */
GlobalDescriptorTable::GlobalDescriptorTable(multiboot_tag_basic_meminfo* meminfo)
: m_null_segment_selector(0, 0, 0), m_unused_segment_selector(0, 0, 0),
  m_code_segment_selector(0, 1024* meminfo -> mem_upper, 0x9A),
  m_data_segment_selector(0, 1024* meminfo -> mem_upper, 0x92),
  m_task_state_segment_selector(0, 1024* meminfo -> mem_upper, 0)

{

    // Create GDT
    uint32_t gdt_t[2];
    gdt_t[0] = sizeof(GlobalDescriptorTable) << 16;
    gdt_t[1] = (uint32_t)this;

    // Tell processor to use this table
    asm volatile("lgdt (%0)": :"p" (((uint8_t *) gdt_t)+2));

}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
}


/**
 * @brief Data Segment Selector
 *
 * @return The data segment selector offset
 */
uint16_t GlobalDescriptorTable::data_segment_selector()
{
    return (uint8_t*)&m_data_segment_selector - (uint8_t*)this;
}

/**
 * @brief Code Segment Selector
 *
 * @return The code segment selector offset
 */
uint16_t GlobalDescriptorTable::code_segment_selector()
{
    return (uint8_t*)&m_code_segment_selector - (uint8_t*)this;
}

/**
 * @brief Task State Segment Selector
 *
 * @return The task state segment selector offset
 */
uint16_t GlobalDescriptorTable::task_state_segment_selector()
{
    return (uint8_t*)&m_task_state_segment_selector - (uint8_t*)this;
}