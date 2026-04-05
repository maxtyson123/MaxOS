/**
 * @file interrupts.cpp
 * @brief Implementation of Interrupt Handling classes
 *
 * @date 10th July 2022
 * @author Max Tyson
 */

#include <hardwarecommunication/interrupts.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;

/**
 * @brief Creates a new interrupt handler and registers it with the interrupt manager
 *
 * @param interrupt_number The interrupt number to handle
 * @param redirect What legacy interrupt to redirect from (-1 for no redirection)
 * @param redirect_index Which index of the legacy interrupt to redirect from
 */
InterruptHandler::InterruptHandler(uint8_t interrupt_number, int64_t redirect, uint64_t redirect_index)
: m_interrupt_number(interrupt_number)
{

	// Get the interrupt manager
	InterruptManager* interrupt_manager = InterruptManager::s_active_interrupt_manager;
	ASSERT(interrupt_manager != nullptr, "No active interrupt manager");

	// Register the handler
	interrupt_manager->set_interrupt_handler(m_interrupt_number, this);

	// Not all interrupts need redirecting
	if (redirect == -1)
		return;

	// Redirect a legacy interrupt to an interrupt the kernel expects
	IOAPIC* io_apic = interrupt_manager->active_apic()->io_apic();
	interrupt_redirect_t temp = {
			.type = (uint8_t) redirect,
			.index = (uint8_t) redirect_index,
			.interrupt = m_interrupt_number,
			.destination = 0x00,
			.flags = 0x00,
			.mask = false,
	};
	io_apic->set_redirect(&temp);
}

/**
 * @brief Destroys the interrupt handler and unregisters it from the interrupt manager
 */
InterruptHandler::~InterruptHandler() {

	// Get the interrupt manager
	InterruptManager* interrupt_manager = InterruptManager::s_active_interrupt_manager;
	if (interrupt_manager == nullptr) return;

	// Remove the handler
	interrupt_manager->set_interrupt_handler(m_interrupt_number, nullptr);
}

/**
 * @brief Handles an interrupt
 */
void InterruptHandler::handle_interrupt() {

}

/**
 * @brief Handles an interrupt and returns the status
 *
 * @param status The status of the CPU
 * @return  The status of the CPU
 */
cpu_status_t* InterruptHandler::handle_interrupt(cpu_status_t* status) {

	// For handlers that don't care about the status
	handle_interrupt();

	// Return the default status
	return status;
}

ExceptionHandler::ExceptionHandler(uint8_t interrupt_number)
: InterruptHandler(interrupt_number)
{
}

ExceptionHandler::~ExceptionHandler() = default;

string ExceptionHandler::get_message(system::cpu_status_t *status) {

	return "Unimplemented message for exception\n";

}


cpu_status_t* ExceptionHandler::handle_interrupt(cpu_status_t *status) {

	// Get the message
	string msg = get_message(status);

	// Try to avoid the panic
	cpu_status_t* can_avoid = CPU::prepare_for_panic(status, msg);
	if (can_avoid != nullptr)
		return can_avoid;

	// Have to panic (must have occurred in kernel)
	CPU::PANIC(msg.c_str(), status);

	// Shouldn't return, fault if this happens
	return nullptr;

}

string InvaildOpCodeHandler::get_message(cpu_status_t *status) {

	return  StringBuilder() << "Invaild opcode executed at 0x" << status->rip << "\n";

}

string GeneralProtectionHandler::get_message(cpu_status_t *status) {

	// Get the core that the fault happened on
	auto core = CPU::executing_core();
	uint64_t core_id = core ? core->id : 0;

	uint64_t error_code = status->error_code;
	return StringBuilder() << "General Protection Fault: (0x" << status->rip << "): " << (error_code & 0x1 ? "Protection-Exception" : "Not a Protection Exception") << " cid: " << core_id << "\n";

}

string PageFaultHandler::get_message(cpu_status_t *status) {

	// Extract the information about the fault
	bool present = (status->error_code & 0x1) != 0;
	bool write = (status->error_code & 0x2) != 0;
	bool user_mode = (status->error_code & 0x4) != 0;
	bool reserved_write = (status->error_code & 0x8) != 0;
	bool instruction_fetch = (status->error_code & 0x10) != 0;
	uint64_t faulting_address;
	asm volatile("movq %%cr2, %0" : "=r" (faulting_address));

	return StringBuilder() << "Page Fault: " << (user_mode ? "user" : "kernel")  << " code at 0x" << status->rip << " tried to " << (write ? "write" : "read") << " address 0x" << faulting_address << " which is " << (present ? "" : "not") << " mapped and " << (reserved_write ? "" : "not") << " reserved. " << " (instruction fetch: " << (instruction_fetch ? "Yes" : "No") << ")\n";
}

cpu_status_t * CPUStopHandler::handle_interrupt(cpu_status_t *status) {

	CPU::halt();

	// Shouldn't return, fault if this happens
	return nullptr;
}

/**
 * @brief Constructs the Interrupt Manager and sets up the interrupt descriptor table (does not activate interrupts)
 */
InterruptManager::InterruptManager() {

	Logger::INFO() << "Setting up Interrupt Manager\n";
	s_active_interrupt_manager = this;

	// Clear the table
	for (auto& descriptor: s_interrupt_descriptor_table) {
		descriptor.address_low_bits = 0;
		descriptor.address_mid_bits = 0;
		descriptor.address_high_bits = 0;
		descriptor.segment_selector = 0;
		descriptor.ist = 0;
		descriptor.flags = 0;
	}

	//Set Up the base interrupts
	set_interrupt_descriptor_table_entry(0x00, &HandleException0x00,		InterruptPrivilegeLevel::KERNEL_ONLY);   // Division by zero
	set_interrupt_descriptor_table_entry(0x01, &HandleException0x01, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Debug
	set_interrupt_descriptor_table_entry(0x02, &HandleException0x02, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Non-maskable interrupt
	set_interrupt_descriptor_table_entry(0x03, &HandleException0x03, 		InterruptPrivilegeLevel::USERSPACE);	 // Breakpoint
	set_interrupt_descriptor_table_entry(0x04, &HandleException0x04, 		InterruptPrivilegeLevel::USERSPACE);	 // Overflow
	set_interrupt_descriptor_table_entry(0x05, &HandleException0x05, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Bound Range Exceeded
	set_interrupt_descriptor_table_entry(0x06, &HandleException0x06, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Invalid Opcode
	set_interrupt_descriptor_table_entry(0x07, &HandleException0x07,		InterruptPrivilegeLevel::KERNEL_ONLY);   // Device Not Available
	set_interrupt_descriptor_table_entry(0x08, &HandleInterruptError0x08,	InterruptPrivilegeLevel::KERNEL_ONLY);   // Double Fault
	set_interrupt_descriptor_table_entry(0x09, &HandleException0x09,		InterruptPrivilegeLevel::KERNEL_ONLY);   // Coprocessor Segment Overrun
	set_interrupt_descriptor_table_entry(0x0A, &HandleInterruptError0x0A, 	InterruptPrivilegeLevel::KERNEL_ONLY);   // Invalid TSS
	set_interrupt_descriptor_table_entry(0x0B, &HandleInterruptError0x0B, 	InterruptPrivilegeLevel::KERNEL_ONLY);   // Segment Not Present
	set_interrupt_descriptor_table_entry(0x0C, &HandleInterruptError0x0C, 	InterruptPrivilegeLevel::KERNEL_ONLY);   // Stack-Segment Fault
	set_interrupt_descriptor_table_entry(0x0D, &HandleInterruptError0x0D, 	InterruptPrivilegeLevel::KERNEL_ONLY);   // General Protection Fault
	set_interrupt_descriptor_table_entry(0x0E, &HandleInterruptError0x0E, 	InterruptPrivilegeLevel::KERNEL_ONLY);   // Page Fault
	set_interrupt_descriptor_table_entry(0x0F, &HandleException0x0F, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x10, &HandleException0x10, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // x87 Floating-Point Exception
	set_interrupt_descriptor_table_entry(0x11, &HandleInterruptError0x11, 	InterruptPrivilegeLevel::KERNEL_ONLY);   // Alignment Check
	set_interrupt_descriptor_table_entry(0x12, &HandleException0x12, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Machine Check
	set_interrupt_descriptor_table_entry(0x13, &HandleException0x13, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // SIMD Floating-Point Exception
	set_interrupt_descriptor_table_entry(0x14, &HandleException0x14, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved: Virtualization Exception
	set_interrupt_descriptor_table_entry(0x15, &HandleException0x15, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x16, &HandleException0x16, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x17, &HandleException0x17, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x18, &HandleException0x18, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x19, &HandleException0x19, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x1A, &HandleException0x1A, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x1B, &HandleException0x1B, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x1C, &HandleException0x1C, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x1D, &HandleException0x1D, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved
	set_interrupt_descriptor_table_entry(0x1E, &HandleException0x1E, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Security Exception
	set_interrupt_descriptor_table_entry(0x1F, &HandleException0x1F, 		InterruptPrivilegeLevel::KERNEL_ONLY);   // Reserved

	// Set up the hardware interrupts
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x00, &HandleInterruptRequest0x00, InterruptPrivilegeLevel::KERNEL_ONLY);   // APIC Timer Interrupt
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x01, &HandleInterruptRequest0x01, InterruptPrivilegeLevel::KERNEL_ONLY);   // Keyboard Interrupt
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x02, &HandleInterruptRequest0x02, InterruptPrivilegeLevel::KERNEL_ONLY);   // PIT Interrupt
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x04, &HandleInterruptRequest0x04, InterruptPrivilegeLevel::KERNEL_ONLY);   // Serial Interrupt
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x0C, &HandleInterruptRequest0x0C, InterruptPrivilegeLevel::KERNEL_ONLY);   // Mouse Interrupt

	// Set up the software interrupts
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x60, &HandleInterruptRequest0x60, InterruptPrivilegeLevel::USERSPACE);   // System Call Interrupt - Privilege Level 3 so that user space can call it
	set_interrupt_descriptor_table_entry(HARDWARE_INTERRUPT_OFFSET + 0x61, &HandleInterruptRequest0x61, InterruptPrivilegeLevel::KERNEL_ONLY);   // Kill this core

	// Tell the processor to use the IDT
	load_current();
}

/**
 * @brief Destroys the Interrupt Manager and deactivates interrupts
 */
InterruptManager::~InterruptManager() {
	deactivate();
}

/**
 * @brief Sets an entry in the Interrupt Descriptor Table
 *
 * @param interrupt  Interrupt number
 * @param handler  Interrupt Handler
 * @param descriptor_privilege_level Descriptor Privilege Level
 */
void InterruptManager::set_interrupt_descriptor_table_entry(uint8_t interrupt, void (* handler)(), InterruptPrivilegeLevel descriptor_privilege_level) {

	// Get the address of the handler and the entry in the IDT
	auto handler_address = (uint64_t) handler;
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
	interrupt_descriptor->flags = 0b1110 | (((uint8_t)descriptor_privilege_level & 0b11) << 5) | (1 << 7);
}

/**
 * @brief Tell the processor to use the current InterruptManager
 */
void InterruptManager::load_current() {

	IDTR idt = {};
	idt.limit = 256 * sizeof(InterruptDescriptor) - 1;
	idt.base = (uint64_t) s_interrupt_descriptor_table;
	asm volatile("lidt %0" : : "m" (idt));
}

/**
 * @brief Activates the interrupt manager and starts interrupts (also deactivates the current interrupt manager)
 */
void InterruptManager::activate() {

	Logger::INFO() << "Activating Interrupts \n";

	// Deactivate the current (old) interrupt manager
	deactivate();

	// Set the current interrupt manager and start interrupts
	s_active_interrupt_manager = this;
	asm("sti");
}

/**
 * @brief Deactivates the interrupt manager and stops interrupts
 */
void InterruptManager::deactivate() {

	// Cant deactivate if it isn't the system one
	if (s_active_interrupt_manager != nullptr)
		return;

	// Prevent interrupts from firing when nothing is set up to handle them
	asm("cli");
	s_active_interrupt_manager = nullptr;
}

/**
 * @brief Handles the interrupt request by passing it to the interrupt manager
 *
 * @param status The current cpu status
 * @return The updated cpu status
 *
 * @todo Exception handlers
 */
cpu_status_t* InterruptManager::HandleInterrupt(cpu_status_t* status) {

	// If there is an interrupt manager handle interrupt
	if (s_active_interrupt_manager != nullptr)
		return s_active_interrupt_manager->handle_interrupt_request(status);

	// CPU Can continue
	return status;
}

/**
 * @brief Sets the interrupt handler for the interrupt
 *
 * @param interrupt The interrupt number
 * @param handler The interrupt handler
 */
void InterruptManager::set_interrupt_handler(uint8_t interrupt, InterruptHandler* handler) {

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

/**
 * @brief Handles the interrupt request by passing it to the appropriate handler
 *
 * @param status The current cpu status
 * @return The updated cpu status
 */
cpu_status_t* InterruptManager::handle_interrupt_request(cpu_status_t* status) {

	// Handle the interrupt
	ASSERT(m_interrupt_handlers[status->interrupt_number] != nullptr, "Interrupt 0x%x not handled\n", status->interrupt_number);
	cpu_status_t* new_status = m_interrupt_handlers[status->interrupt_number]->handle_interrupt(status);

	// Send the EOI to the APIC
	if (HARDWARE_INTERRUPT_OFFSET <= status->interrupt_number && status->interrupt_number < HARDWARE_INTERRUPT_OFFSET + 16)
		CPU::executing_core()->local_apic->send_eoi();

	// Return the status
	return new_status;
}

/**
 * @brief Sets the APIC
 *
 * @param apic The APIC
 */
void InterruptManager::set_apic(AdvancedProgrammableInterruptController* apic) {

	m_apic = apic;
}


ExceptionHandlers::ExceptionHandlers() = default;
ExceptionHandlers::~ExceptionHandlers() = default;

/**
 * @brief Handles a page fault
 *
 * @param status The cpu status
 * @return The updated cpu status (won't return if it panics)
 */
cpu_status_t* InterruptManager::page_fault(cpu_status_t* status) {

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
InterruptManager* InterruptManager::active_interrupt_manager() {

	return s_active_interrupt_manager;
}