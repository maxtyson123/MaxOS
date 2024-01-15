//
// Created by 98max on 7/10/2022.
//

#include <hardwarecommunication/interrupts.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;

// Define the static variables

InterruptManager* InterruptManager::s_active_interrupt_manager = 0;
OutputStream* InterruptManager::s_error_messages = 0;
GateDescriptor InterruptManager::s_interrupt_descriptor_table[256];

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



InterruptManager::InterruptManager(uint16_t hardware_interrupt_offset, system::GlobalDescriptorTable*global_descriptor_table,ThreadManager*thread_manager,  OutputStream* handler)
: common::InputStream(handler),
  m_hardware_interrupt_offset(hardware_interrupt_offset),
  m_thread_manager(thread_manager),
  pic_master_command_port(0x20),
  pic_master_data_port(0x21),
  pic_slave_command_port(0xA0),
  pic_slave_data_port(0xA1)
{

    uint32_t code_segment = global_descriptor_table->code_segment_selector();

    // By default ignore all interrupts so any un handled interrupts wont cause a fault
    const uint8_t IDT_INTERRUPT_GATE = 0xE;
    for(uint8_t i = 255; i > 0; --i)
    {
      set_interrupt_descriptor_table_entry(i, code_segment, &InterruptIgnore, 0, IDT_INTERRUPT_GATE);
      m_interrupt_handlers[i] = 0;
    }

    // First is also clear
    set_interrupt_descriptor_table_entry(0, code_segment, &InterruptIgnore, 0,IDT_INTERRUPT_GATE);
    m_interrupt_handlers[0] = 0;



    //Set Up the base interrupts
    set_interrupt_descriptor_table_entry(0x00, code_segment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);   //Division by zero
    set_interrupt_descriptor_table_entry(0x01, code_segment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);   //Single-step interrupt (see trap flag)
    set_interrupt_descriptor_table_entry(0x02, code_segment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);   //NMI
    set_interrupt_descriptor_table_entry(0x03, code_segment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);   //Breakpoint (which benefits from the shorter 0xCC encoding of INT 3)
    set_interrupt_descriptor_table_entry(0x04, code_segment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);   //Overflow
    set_interrupt_descriptor_table_entry(0x05, code_segment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);   //Bound Range Exceeded
    set_interrupt_descriptor_table_entry(0x06, code_segment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);   //Invalid Opcode
    set_interrupt_descriptor_table_entry(0x07, code_segment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);   //Coprocessor not available
    set_interrupt_descriptor_table_entry(0x08, code_segment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);   //Double Fault
    set_interrupt_descriptor_table_entry(0x09, code_segment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);   //Coprocessor Segment Overrun (386 or earlier only)
    set_interrupt_descriptor_table_entry(0x0A, code_segment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);   //Invalid Task State Segment
    set_interrupt_descriptor_table_entry(0x0B, code_segment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);   //Segment not present
    set_interrupt_descriptor_table_entry(0x0C, code_segment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);   //Stack Segment Fault
    set_interrupt_descriptor_table_entry(0x0D, code_segment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);   //General Protection Fault
    set_interrupt_descriptor_table_entry(0x0E, code_segment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);   //Page Fault
    set_interrupt_descriptor_table_entry(0x0F, code_segment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x10, code_segment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);   //x87 Floating Point Exception
    set_interrupt_descriptor_table_entry(0x11, code_segment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);   //Alignment Check
    set_interrupt_descriptor_table_entry(0x12, code_segment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);   //Machine Check
    set_interrupt_descriptor_table_entry(0x13, code_segment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);   //SIMD Floating-Point Exception
    set_interrupt_descriptor_table_entry(0x14, code_segment, &HandleException0x14, 0, IDT_INTERRUPT_GATE);   //Virtualization Exception
    set_interrupt_descriptor_table_entry(0x15, code_segment, &HandleException0x15, 0, IDT_INTERRUPT_GATE);   //Control Protection Exception
    set_interrupt_descriptor_table_entry(0x16, code_segment, &HandleException0x16, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x17, code_segment, &HandleException0x17, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x18, code_segment, &HandleException0x18, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x19, code_segment, &HandleException0x19, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x1A, code_segment, &HandleException0x1A, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x1B, code_segment, &HandleException0x1B, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x1C, code_segment, &HandleException0x1C, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x1D, code_segment, &HandleException0x1D, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x1E, code_segment, &HandleException0x1E, 0, IDT_INTERRUPT_GATE);   //reserved
    set_interrupt_descriptor_table_entry(0x1F, code_segment, &HandleException0x1F, 0, IDT_INTERRUPT_GATE);   //reserved


    //Set up the hardware interrupts (offset by 0x20) //Ranges 0x20 - 0x30
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x00, code_segment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);  //0x20 - Default PIC interval   / Timer
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x01, code_segment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);  //0x21 - Keyboard
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x02, code_segment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);  //0x22 - Cascade (used internally by the two PICs. never raised)
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x03, code_segment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);  //0x23 - COM2, COM4
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x04, code_segment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);  //0x24 - COM1, COM3
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x05, code_segment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);  //0x25 - LPT2, LPT4
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x06, code_segment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);  //0x26 - LPT1
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x07, code_segment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);  //0x27 - Floppy Disk
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x08, code_segment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);  //0x28 - CMOS Real Time Clock
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x09, code_segment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);  //0x29 - Free for peripherals / legacy SCSI / NIC
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0A, code_segment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);  //0x2A - Free for peripherals / SCSI / NIC
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0B, code_segment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);  //0x2B - Free for peripherals / SCSI / NIC
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0C, code_segment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);  //0x0C - Mouse
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0D, code_segment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);  //0x2D - FPU / Coprocessor / Inter-processor
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0E, code_segment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);  //0x2E - Primary ATA Hard Disk
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0F, code_segment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);  //0x2F - Secondary ATA Hard Disk
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x60, code_segment, &HandleInterruptRequest0x80, 0, IDT_INTERRUPT_GATE);  //0x80 - Sys calls

    //Send Initialization Control Words
    pic_master_command_port.write(0x11);
    pic_slave_command_port.write(0x11);

    // Remap the PIC to use the hardware interrupt offset
    pic_master_data_port.write(hardware_interrupt_offset);
    pic_slave_data_port.write(hardware_interrupt_offset + 8);

    //Tell PICs their roles
    pic_master_data_port.write(0x04);  //Master
    pic_slave_data_port.write(0x02);   //Slave

    //Tell PICS that they are in 8086 mode
    pic_master_data_port.write(0x01);
    pic_slave_data_port.write(0x01);

    // Clear the interrupt mask to enable all interrupts
    pic_master_data_port.write(0x00);
    pic_slave_data_port.write(0x00);

    //Tell the processor to use the IDT
    InterruptDescriptorTablePointer idt_pointer;
    idt_pointer.size  = 256*sizeof(GateDescriptor) - 1;
    idt_pointer.base  = (uint32_t)s_interrupt_descriptor_table;
    asm volatile("lidt %0" : : "m" (idt_pointer));
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
 * @param DescriptorPrivilegeLevel Descriptor Privilege Level
 * @param descriptor_type  Descriptor Type
 */
void InterruptManager::set_interrupt_descriptor_table_entry(uint8_t interrupt, uint16_t code_segment_selector_offset, void (*handler)(), uint8_t DescriptorPrivilegeLevel, uint8_t descriptor_type)
{
  // Store the handler function in the IDT
  s_interrupt_descriptor_table[interrupt].handler_address_low_bits = ((uint32_t) handler) & 0xFFFF;
  s_interrupt_descriptor_table[interrupt].handler_address_high_bits = (((uint32_t) handler) >> 16) & 0xFFFF;
  s_interrupt_descriptor_table[interrupt].gdt_code_segment_selector = code_segment_selector_offset;

  // Set the access
  const uint8_t IDT_DESC_PRESENT = 0x80;
  s_interrupt_descriptor_table[interrupt].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | descriptor_type;
  s_interrupt_descriptor_table[interrupt].reserved = 0;
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
 * @param interruptNumber The interrupt number
 * @param esp The stack pointer
 * @return The stack pointer
 */
uint32_t InterruptManager::HandleInterrupt(uint8_t interrupt, uint32_t esp)
{

    // If there is an active interrupt manager, handle the interrupt
    if(s_active_interrupt_manager != 0)
        return s_active_interrupt_manager->handle_interrupt_request(interrupt, esp);

    // CPU can continue
    return esp;
}

/**
 * @brief Handles the interrupt request and runs the interrupt handler if there is one
 *
 * @param interruptNumber The interrupt number
 * @param esp The stack pointer
 * @return The stack pointer
 */
uint32_t InterruptManager::handle_interrupt_request(uint8_t interrupt, uint32_t esp)
{

    // If there is an interrupt handler, run it
    if(m_interrupt_handlers[interrupt] != 0){
      m_interrupt_handlers[interrupt]->handle_interrupt();
    }
    else if(interrupt != 0x20){

        switch (interrupt) {

            case 0x00: s_error_messages->write("[ERROR] Divide by zero  (int 0x00)"); break;
            case 0x01: s_error_messages->write("[ERROR] Single-step interrupt (int 0x01)"); break;
            case 0x02: s_error_messages->write("[ERROR] Non maskable interrupt (int 0x02)"); break;
            case 0x03: s_error_messages->write("[ERROR] Breakpoint (int 0x03)"); break;
            case 0x04: s_error_messages->write("[ERROR] Overflow (int 0x04)"); break;
            case 0x05: s_error_messages->write("[ERROR] Bounds check  (int 0x05)"); break;
            case 0x06: s_error_messages->write("[ERROR] Invalid opcode  (int 0x06)"); break;
            case 0x07: s_error_messages->write("[ERROR] Coprocessor not available  (int 0x07)"); break;
            case 0x08: s_error_messages->write("[ERROR] Double fault (int 0x08)"); break;
            case 0x09: s_error_messages->write("[ERROR] Coprocessor segment overrun (int 0x09)"); break;
            case 0x0A: s_error_messages->write("[ERROR] Invalid TSS (int 0x0A)"); break;
            case 0x0B: s_error_messages->write("[ERROR] Segment not present (int 0x0B)"); break;
            case 0x0C: s_error_messages->write("[ERROR] Stack segment fault (int 0x0C)"); break;
            case 0x0D: s_error_messages->write("[ERROR] General protection fault (int 0x0D)"); break;
            case 0x0E: s_error_messages->write("[ERROR] Page fault (int 0x0E)"); break;
            case 0x0F: s_error_messages->write("[INFO] Reserved (int 0x0F)"); break;
            case 0x10: s_error_messages->write("[ERROR] x87 FPU floating point error (int 0x10)"); break;
            case 0x11: s_error_messages->write("[INFO] Alignment check (int 0x11)"); break;
            case 0x12: s_error_messages->write("[INFO] Machine check (int 0x12)"); break;
            case 0x13: s_error_messages->write("[ERROR] SIMD floating point exception (int 0x13)"); break;
            case 0x14: s_error_messages->write("[ERROR] Virtualization exception (int 0x14)"); break;
            case 0x15: s_error_messages->write("[INFO] Reserved (int 0x15)"); break;
            case 0x16: s_error_messages->write("[INFO] Reserved (int 0x16)"); break;
            case 0x17: s_error_messages->write("[INFO] Reserved (int 0x17)"); break;
            case 0x18: s_error_messages->write("[INFO] Reserved (int 0x18)"); break;
            case 0x19: s_error_messages->write("[INFO] Reserved (int 0x19)"); break;
            case 0x1A: s_error_messages->write("[INFO] Reserved (int 0x1A)"); break;
            case 0x1B: s_error_messages->write("[INFO] Reserved (int 0x1B)"); break;
            case 0x1C: s_error_messages->write("[INFO] Reserved (int 0x1C)"); break;
            case 0x1D: s_error_messages->write("[INFO] Reserved (int 0x1D)"); break;
            case 0x1E: s_error_messages->write("[INFO] Reserved (int 0x1E)"); break;
            case 0x1F: s_error_messages->write("[INFO] Reserved (int 0x1F)"); break;

            default:
              s_error_messages->write("UNHANDLED INTERRUPT 0x");
              s_error_messages->write_hex(interrupt);
              s_error_messages->write(" ");
                break;

        }
    }

    //Timer interrupt for m_tasks
    if(interrupt == m_hardware_interrupt_offset)
    {
        esp = (uint32_t)m_thread_manager->schedule((CPUState*)esp);
    }

    // Acknowledge the interrupt (if it is a hardware interrupt)
    if(m_hardware_interrupt_offset <= interrupt && interrupt < m_hardware_interrupt_offset +16)
    {
      pic_master_command_port.write(0x20);

        // Answer the slave PIC if it was the one that sent the interrupt
        if(0x28 <= interrupt)
          pic_slave_command_port.write(0x20);
    }

    return esp;
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