//
// Created by 98max on 7/10/2022.
//

#include <hardwarecommunication/interrupts.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;


///__Handler__

InterruptHandler::InterruptHandler(uint8_t interrupt_number, int64_t redirect, uint64_t redirect_index)
: m_interrupt_number(interrupt_number)
{

    // Get the interrupt manager
    InterruptManager* interrupt_manager = InterruptManager::s_active_interrupt_manager;

    // If there is no interrupt manager, nothing can be done
    ASSERT(interrupt_manager != nullptr, "No active interrupt manager");

    // Set the handler in the array
    interrupt_manager -> set_interrupt_handler(m_interrupt_number, this);

    // If there is a redirect, set it
    if(redirect == -1) return;

    // Get the IO-APIC
    IOAPIC* io_apic = interrupt_manager -> active_apic() -> io_apic();

    // Register the driver
    interrupt_redirect_t mouseRedirect = {
        .type = (uint8_t)redirect,
        .index = (uint8_t)redirect_index,
        .interrupt = m_interrupt_number,
        .destination = 0x00,
        .flags = 0x00,
        .mask = false,
    };
    io_apic -> set_redirect(&mouseRedirect);
}


InterruptHandler::~InterruptHandler(){

  // Get the interrupt manager
  InterruptManager* interrupt_manager = InterruptManager::s_active_interrupt_manager;

  // If there is no interrupt manager, no need to remove the handler
  if(interrupt_manager == nullptr) return;

  // Remove the handler
  interrupt_manager -> set_interrupt_handler(m_interrupt_number, nullptr);
}

/**
 * @brief Handles an interrupt
 */
void InterruptHandler::handle_interrupt() {

}

/**
 * @brief Handles an interrupt and returns the status
 * @param status The status of the CPU
 * @return  The status of the CPU
 */
system::cpu_status_t* InterruptHandler::handle_interrupt(system::cpu_status_t *status) {

  // For handlers that don't care about the status
  handle_interrupt();

  // Return the status
  return status;

}

///__Manger__



InterruptManager::InterruptManager()
{

     // Full the table of interrupts with 0
     for(auto& descriptor : s_interrupt_descriptor_table) {
       descriptor.address_low_bits = 0;
       descriptor.address_mid_bits = 0;
       descriptor.address_high_bits = 0;
       descriptor.segment_selector = 0;
       descriptor.ist = 0;
       descriptor.flags = 0;
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
    set_interrupt_descriptor_table_entry(s_hardware_interrupt_offset + 0x00, &HandleInterruptRequest0x00, 0);   // APIC Timer Interrupt
    set_interrupt_descriptor_table_entry(s_hardware_interrupt_offset + 0x01, &HandleInterruptRequest0x01, 0);   // Keyboard Interrupt
    set_interrupt_descriptor_table_entry(s_hardware_interrupt_offset + 0x02, &HandleInterruptRequest0x02, 0);   // PIT Interrupt
    set_interrupt_descriptor_table_entry(s_hardware_interrupt_offset + 0x0C, &HandleInterruptRequest0x0C, 0);   // Mouse Interrupt

    // Set up the system call interrupt
    set_interrupt_descriptor_table_entry(s_hardware_interrupt_offset + 0x60, &HandleInterruptRequest0x60, 3);   // System Call Interrupt - Privilege Level 3 so that user space can call it

    //Tell the processor to use the IDT
    IDTR idt = {};
    idt.limit = 256 * sizeof(InterruptDescriptor) - 1;
    idt.base = (uint64_t)s_interrupt_descriptor_table;
    asm volatile("lidt %0" : : "m" (idt));

    // Set the active interrupt manager
    s_active_interrupt_manager = this;
}

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
  auto handler_address = (uint64_t)handler;
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
    if(s_active_interrupt_manager != nullptr)
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
      s_active_interrupt_manager = nullptr;
      asm("cli");
    }
}

/**
 * @brief Handles the interrupt request by passing it to the interrupt manager
 *
 * @param status The current cpu status
 * @return The updated cpu status
 */
system::cpu_status_t* InterruptManager::HandleInterrupt(system::cpu_status_t *status) {

  // Fault Handlers
  switch (status->interrupt_number) {

    case 0x7:
      _kpanicf("Device Not Available: FPU Not Enabled\n");
      CPU::prepare_for_panic(status);
      CPU::PANIC("See above message for more information", status);
      break;

    case 0x0D:
          return general_protection_fault(status);

    case 0x0E:
      return page_fault(status);
    }

  // If there is an interrupt manager handle interrupt
  if(s_active_interrupt_manager != nullptr)
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
    return s_hardware_interrupt_offset;
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
  m_interrupt_handlers[interrupt] = nullptr;
}

cpu_status_t* InterruptManager::handle_interrupt_request(cpu_status_t* status) {

  // Where to go afterward
  cpu_status_t* new_status = status;

  // If there is an interrupt manager, handle the interrupt
  if(m_interrupt_handlers[status -> interrupt_number] != nullptr)
    new_status = m_interrupt_handlers[status -> interrupt_number]->handle_interrupt(status);
  else
    _kprintf("Unhandled Interrupt 0x%x\n", status->interrupt_number);

  // Send the EOI to the APIC
  if(s_hardware_interrupt_offset <= status->interrupt_number && status->interrupt_number < s_hardware_interrupt_offset + 16)
    m_apic->local_apic()->send_eoi();

  // Return the status
  return new_status;
}

void InterruptManager::set_apic(AdvancedProgrammableInterruptController *apic) {
  m_apic = apic;
}

cpu_status_t* InterruptManager::page_fault(system::cpu_status_t *status) {
  bool present = (status ->error_code & 0x1) != 0;         // Bit 0: Page present flag
  bool write = (status ->error_code & 0x2) != 0;           // Bit 1: Write operation flag
  bool user_mode = (status ->error_code & 0x4) != 0;       // Bit 2: User mode flag
  bool reserved_write = (status ->error_code & 0x8) != 0;  // Bit 3: Reserved bit write flag
  bool instruction_fetch = (status ->error_code & 0x10) != 0; // Bit 4: Instruction fetch flag (on some CPUs)
  uint64_t faulting_address;
  asm volatile("movq %%cr2, %0" : "=r" (faulting_address));

  cpu_status_t* can_avoid = CPU::prepare_for_panic(status);
  if(can_avoid != nullptr)
    return can_avoid;

  _kpanicf("Page Fault (0x%x): present: %s, write: %s, user-mode: %s, reserved write: %s, instruction fetch: %s\n", faulting_address, (present ? "Yes" : "No"), (write ? "Yes" : "No"), (user_mode ? "Yes" : "No"), (reserved_write ? "Yes" : "No"), (instruction_fetch ? "Yes" : "No"));
  CPU::PANIC("See above message for more information", status);

  // Probably should never get here
  return status;
}

/**
 * @brief Handles a general protection fault
 * @param status The cpu status
 */
cpu_status_t* InterruptManager::general_protection_fault(system::cpu_status_t *status) {
    uint64_t error_code = status->error_code;

    // Try to avoid the panic
    cpu_status_t* can_avoid = CPU::prepare_for_panic(status);
    if(can_avoid != nullptr)
      return can_avoid;

    // Have to panic
    _kpanicf("General Protection Fault (0x%x): %s\n", status -> rip, (error_code & 0x1) ? "Protection-Exception" : "Not a Protection Exception");
    CPU::PANIC("See above message for more information", status);

    // Probably should never get here
    return status;
}

/**
 * @brief Gets the APIC
 *
 * @return The APIC
 */
AdvancedProgrammableInterruptController* InterruptManager::active_apic() {
        return m_apic;
}

/**
 * @brief Gets the active interrupt manager
 *
 * @return The active interrupt manager
 */
InterruptManager *InterruptManager::active_interrupt_manager() {
  return s_active_interrupt_manager;
}
