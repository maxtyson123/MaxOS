//
// Created by 98max on 7/10/2022.
//

#include <hardwarecommunication/interrupts.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;

// Define the static variables

InterruptManager* InterruptManager::s_active_interrupt_manager = 0;
OutputStream* InterruptManager::s_error_messages = 0;
InterruptDescriptor InterruptManager::s_interrupt_descriptor_table[256];

///__Handler__

InterruptHandler::InterruptHandler(uint8_t interrupt_number, InterruptManager* interrupt_manager)
: m_interrupt_number(interrupt_number),
  m_interrupt_manager(interrupt_manager)
{
    // Set the handler in the array
    m_interrupt_manager->set_interrupt_handler(interrupt_number, this);
}

InterruptHandler::~InterruptHandler(){

    // Unset the handler in the array
    if(this->m_interrupt_manager != 0)
      this->m_interrupt_manager->remove_interrupt_handler(m_interrupt_number);

}

void InterruptHandler::handle_interrupt() {

}



///__Manger__



InterruptManager::InterruptManager(uint16_t hardware_interrupt_offset, OutputStream* handler)
: common::InputStream(handler),
  m_hardware_interrupt_offset(hardware_interrupt_offset)
{

     // Full the table of interrupts with 0
     for(uint16_t i = 0; i < 256; i++) {
        s_interrupt_descriptor_table[i].address_low_bits = 0;
        s_interrupt_descriptor_table[i].address_mid_bits = 0;
        s_interrupt_descriptor_table[i].address_high_bits = 0;
        s_interrupt_descriptor_table[i].segment_selector = 0;
        s_interrupt_descriptor_table[i].ist = 0;
        s_interrupt_descriptor_table[i].flags = 0;
     }

     //Set Up the base interrupts
    set_interrupt_descriptor_table_entry(0x00, &HandleException0x00, 0);   // Division by zero
    set_interrupt_descriptor_table_entry(0x01, &HandleException0x01, 0);   // Debug
    set_interrupt_descriptor_table_entry(0x02, &HandleException0x02, 0);   // Non-maskable interrupt
    set_interrupt_descriptor_table_entry(0x03, &HandleException0x03, 0);   // Breakpoint
    set_interrupt_descriptor_table_entry(0x04, &HandleException0x04, 0);   // Overflow
    set_interrupt_descriptor_table_entry(0x05, &HandleException0x05, 0);   // Bound Range Exceeded
    set_interrupt_descriptor_table_entry(0x06, &HandleException0x06, 0);   // Invalid Opcode
    set_interrupt_descriptor_table_entry(0x06, &HandleException0x07, 0);   // Device Not Available
    set_interrupt_descriptor_table_entry(0x08, &HandleInterruptError0x08, 0);   // Double Fault
    set_interrupt_descriptor_table_entry(0x09, &HandleException0x09, 0);   // Coprocessor Segment Overrun
    set_interrupt_descriptor_table_entry(0x0A, &HandleInterruptError0x0A, 0);   // Invalid TSS
    set_interrupt_descriptor_table_entry(0x0B, &HandleInterruptError0x0B, 0);   // Segment Not Present
    set_interrupt_descriptor_table_entry(0x0C, &HandleInterruptError0x0C, 0);   // Stack-Segment Fault
    set_interrupt_descriptor_table_entry(0x0D, &HandleInterruptError0x0D, 0);   // General Protection Fault
    set_interrupt_descriptor_table_entry(0x0E, &HandleInterruptError0x0E, 0);   // Page Fault
    set_interrupt_descriptor_table_entry(0x0F, &HandleException0x0F, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x10, &HandleException0x10, 0);   // x87 Floating-Point Exception
    set_interrupt_descriptor_table_entry(0x11, &HandleInterruptError0x11, 0);   // Alignment Check
    set_interrupt_descriptor_table_entry(0x12, &HandleException0x12, 0);   // Machine Check
    set_interrupt_descriptor_table_entry(0x13, &HandleException0x13, 0);   // SIMD Floating-Point Exception
    set_interrupt_descriptor_table_entry(0x14, &HandleException0x14, 0);   // Reserved: Virtualization Exception
    set_interrupt_descriptor_table_entry(0x15, &HandleException0x15, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x16, &HandleException0x16, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x17, &HandleException0x17, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x18, &HandleException0x18, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x19, &HandleException0x19, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x1A, &HandleException0x1A, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x1B, &HandleException0x1B, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x1C, &HandleException0x1C, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x1D, &HandleException0x1D, 0);   // Reserved
    set_interrupt_descriptor_table_entry(0x1E, &HandleException0x1E, 0);   // Security Exception
    set_interrupt_descriptor_table_entry(0x1F, &HandleException0x1F, 0);   // Reserved

    // Set up the hardware interrupts
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x00, &HandleInterruptRequest0x00, 0);   // Advanced Programmable Interrupt Controller (APIC) Timer Interrupt
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x01, &HandleInterruptRequest0x01, 0);   // Keyboard Interrupt
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x02, &HandleInterruptRequest0x02, 0);   // Cascade (used internally by the two PICs. never raised)
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0C, &HandleInterruptRequest0x0C, 0);   // Mouse Interrupt

    // Set up the system call interrupt
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x60, &HandleInterruptRequest0x60, 0);   // System Call Interrupt

    //Tell the processor to use the IDT
    IDTR idt;
    idt.limit = 256 * sizeof(InterruptDescriptor) - 1;
    idt.base = (uint64_t)s_interrupt_descriptor_table;
    asm volatile("lidt %0" : : "m" (idt));
};

InterruptManager::~InterruptManager()
{
  deactivate();
}


/**
 * @brief Sets an entry in the Interrupt Descriptor Table
 *
 * @param interrupt  Interrupt number
 * @param code_segment_selector_offset  Code segment
 * @param handler  Interrupt Handler
 * @param descriptor_privilege_level Descriptor Privilege Level
 * @param descriptor_type  Descriptor Type
 */
void InterruptManager::set_interrupt_descriptor_table_entry(uint8_t interrupt, void (*handler)(), uint8_t descriptor_privilege_level)
{

  // Get the address of the handler and the entry in the IDT
  uint64_t handler_address = (uint64_t)handler;
  InterruptDescriptor* interrupt_descriptor = &s_interrupt_descriptor_table[interrupt];

  // Set the handler address
  interrupt_descriptor->address_low_bits = handler_address & 0xFFFF;
  interrupt_descriptor->address_mid_bits = (handler_address >> 16) & 0xFFFF;
  interrupt_descriptor->address_high_bits = (handler_address >> 32) & 0xFFFFFFFF;

  // Set the kernel code segment offset
  interrupt_descriptor->segment_selector = 0x08;

  // Disable IST
  interrupt_descriptor->ist = 0;

  // Set the flags (Trap Gate, Present and the Descriptor Privilege Level)
  interrupt_descriptor->flags = 0b1110 | ((descriptor_privilege_level & 0b11) << 5) | (1 << 7);
}


/**
 * @brief Activates the interrupt manager and starts interrupts (also deactivates the current interrupt manager)
 */
void InterruptManager::activate() {

    // Deactivate the current interrupt manager
    if(s_active_interrupt_manager != 0)
      s_active_interrupt_manager->deactivate();

    // Set the current interrupt manager and start interrupts
    s_active_interrupt_manager = this;
    asm("sti");
}

/**
 * @brief Deactivates the interrupt manager and stops interrupts
 */
void InterruptManager::deactivate()
{

    // If this is the active interrupt manager, deactivate it and stop interrupts
    if(s_active_interrupt_manager == this){
      s_active_interrupt_manager = 0;
      asm("cli");
    }
}

/**
 * @brief Handles the interrupt request by passing it to the interrupt manager
 *
 * @param status The current stack pointer
 * @return The stack pointer
 */
system::cpu_status_t* InterruptManager::HandleInterrupt(system::cpu_status_t *status) {

  ASSERT(false, "Interupt number 0x%x, Code: 0x%x", status->interrupt_number, status->error_code);

  // If there is an interrupt manager handle interrupt
  if(s_active_interrupt_manager != 0)
    return s_active_interrupt_manager->handle_interrupt_request(status);

  // CPU Can continue
  return status;
}

/**
 * @brief Returns the offset of the hardware interrupt
 *
 * @return The offset of the hardware interrupt
 */
uint16_t InterruptManager::hardware_interrupt_offset() {
    return m_hardware_interrupt_offset;
}

/**
 * @brief Sets the interrupt handler for the interrupt
 *
 * @param interrupt The interrupt number
 * @param handler The interrupt handler
 */
void InterruptManager::set_interrupt_handler(uint8_t interrupt, InterruptHandler *handler) {
  m_interrupt_handlers[interrupt] = handler;
}

/**
 * @brief Removes the interrupt handler for the interrupt
 *
 * @param interrupt The interrupt number
 */
void InterruptManager::remove_interrupt_handler(uint8_t interrupt) {
  m_interrupt_handlers[interrupt] = 0;
}

static const char* exceptions[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "Security Exception",
    "RESERVED"
};

cpu_status_t* InterruptManager::handle_interrupt_request(cpu_status_t* status) {

  _kprintf("Interrupt: 0x%x\n", status->interrupt_number);

  // If there is an interrupt manager, handle the interrupt
  if(m_interrupt_handlers[status -> interrupt_number] != 0)
      m_interrupt_handlers[status -> interrupt_number]->handle_interrupt();

  else if(status->interrupt_number < m_hardware_interrupt_offset){
    _kprintf("Exception: %s, Error Code: 0x%x\n", exceptions[status->interrupt_number], status->error_code);
    CPU::stack_trace(10);
  }


  else
    _kprintf("Unhandled Interrupt: 0x%x\n", status->interrupt_number);

  // Debug the General Protection Fault
  if(status->interrupt_number == 0x0D) {

    // Define masks for each field
    uint32_t E_MASK = 0b10000000000000000000000000000000;
    uint32_t Tbl_MASK = 0b01100000000000000000000000000000;
    uint32_t Index_MASK = 0b00011111111111110000000000000000;

    // Use bit shifting and masking to extract values
    int E = (status -> error_code & E_MASK) >> 31;
    int Tbl = (status -> error_code & Tbl_MASK) >> 29;
    int Index = (status -> error_code & Index_MASK) >> 16;

    // If bit 0 is set, the exception was caused by external event
    _kprintf("General Protection Fault: External Event: %s\n", (E) ? "Yes" : "No");

    switch(Tbl) {
      case 0b00: _kprintf("General Protection Fault: Table: GDT\n"); break;
      case 0b01: _kprintf("General Protection Fault: Table: IDT\n"); break;
      case 0b10: _kprintf("General Protection Fault: Table: LDT\n"); break;
      case 0b11: _kprintf("General Protection Fault: Table: IDT\n"); break;
    }

    // Find the selector index (next 13 bits)
    _kprintf("General Protection Fault: Selector Index: 0x%x\n", Index);

    // Hang
    while(true);
  }


  //TODO: Send SMP interrupt
  //todo: send eoi

  // Return the status
  return status;
}