//
// Created by 98max on 18/01/2024.
//
#include <hardwarecommunication/apic.h>
#include <common/logger.h>
#include <hardwarecommunication/interrupts.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::memory;

LocalAPIC::LocalAPIC() {

	// Get the APIC base address
	uint64_t msr_info = CPU::read_msr(0x1B);
	m_apic_base = msr_info & 0xFFFFF000;
	PhysicalMemoryManager::s_current_manager->reserve(m_apic_base);

	// Check if the APIC supports x2APIC
	uint32_t ignored, xleaf, x2leaf;
	CPU::cpuid(0x01, &ignored, &ignored, &x2leaf, &xleaf);

	if (x2leaf & (1 << 21)) {

		// Enable x2APIC
		m_x2apic = true;
		msr_info |= (1 << 10);
		CPU::write_msr(0x1B, msr_info);
		Logger::DEBUG() << "CPU supports x2APIC\n";

	} else if (xleaf & (1 << 9)) {

		m_x2apic = false;
		Logger::DEBUG() << "CPU supports xAPIC\n";

		// Map the APIC base address to the higher half
		m_apic_base_high = (uint64_t) PhysicalMemoryManager::to_io_region(m_apic_base);
		PhysicalMemoryManager::s_current_manager->map((physical_address_t*) m_apic_base,
													  (virtual_address_t*) m_apic_base_high, Write | Present);
		Logger::DEBUG() << "APIC Base: phy=0x" << m_apic_base << ", virt=0x" << m_apic_base_high << "\n";

	} else {
		ASSERT(false, "CPU does not support xAPIC");
	}

	// Get information about the APIC
	uint32_t spurious_vector = read(0xF0);
	bool is_enabled = msr_info & (1 << 11);
	bool is_bsp = msr_info & (1 << 8);
	Logger::DEBUG() << "APIC: boot processor: " << (is_bsp ? "Yes" : "No") << ", enabled (globally): " << (is_enabled ? "Yes" : "No") << " Spurious Vector: 0x" << (uint64_t) (spurious_vector & 0xFF) << "\n";


	if (!is_enabled) {
		Logger::WARNING() << "APIC is not enabled\n";
		return;
	}

	// Enable the APIC
	write(0xF0, (1 << 8) | 0x100);
	Logger::DEBUG() << "APIC Enabled\n";
}

LocalAPIC::~LocalAPIC() = default;

/**
 * @brief Read a value from the apic register using the MSR or memory I/O depending on the local apic version
 *
 * @param reg The register to read
 * @return The value of the register
 */
uint32_t LocalAPIC::read(uint32_t reg) const {

	// If x2APIC is enabled I/O is done through the MSR
	if (m_x2apic)
		return (uint32_t) CPU::read_msr((reg >> 4) + 0x800);

	return *(volatile uint32_t*) ((uintptr_t) m_apic_base_high + reg);
}

/**
 * @brief Write a value to the apic register using the MSR or memory I/O depending on the local apic version
 *
 * @param reg The register to write to
 * @param value The value to write
 */
void LocalAPIC::write(uint32_t reg, uint32_t value) const {

	// If x2APIC is enabled I/O is done through the MSR
	if (m_x2apic)
		CPU::write_msr((reg >> 4) + 0x800, value);

	// Default to memory I/O
	*(volatile uint32_t*) ((uintptr_t) m_apic_base_high + reg) = value;
}

/**
 * @brief Get the id of the local apic
 *
 * @return The id of the local apic
 */
uint32_t LocalAPIC::id() const {

	// Read the id
	uint32_t id = read(0x20);

	// Return the id
	return m_x2apic ? id : (id >> 24);
}

/**
 * @brief Acknowledge that the interrupt has been handled, allowing the PIC to process the next interrupt
 */
void LocalAPIC::send_eoi() const {

	write(0xB0, 0);
}

/**
 * @brief Send the init IPI to another apic
 *
 * @param apic_id The id of the apic to send to
 */
void LocalAPIC::send_init(uint8_t apic_id, bool assert) {

	uint32_t icr_low = 0;

	// Delivery mode = INIT (101b at bits 8-10)
	icr_low |= (0b101 << 8);

	// Level (bit 14): 1 = assert, 0 = de-assert
	if (assert)
		icr_low |= (1 << 14);

	// Trigger mode: Level = 1
	icr_low |= (1 << 15);

	if (!m_x2apic) {

		// Select target core
		write(0x310, apic_id << 24);

		// Send INIT (Delivery mode = INIT, Level = 1)
		write(0x300, icr_low);

		// Wait for delivery
		while (read(0x300) & (1 << 12))
			asm volatile("pause");

	} else {

		// x2APIC
		CPU::write_msr(0x830, apic_id << 32 | icr_low);
	}
}

/**
 * @brief Send the start up IPI to another apic
 *
 * @param apic_id The apic to send it to
 * @param vector Where to start executing
 */
void LocalAPIC::send_startup(uint8_t apic_id, uint8_t vector) {

	if (!m_x2apic) {

		// Select target core
		write(0x310, apic_id << 24);

		// Send SIPI (Delivery mode = STARTUP)
		write(0x300, 0x4600 | vector);

		// Wait for delivery
		while (read(0x300) & (1 << 12))
		 	asm volatile("pause");

	} else {

		// x2APIC
		CPU::write_msr(0x831, apic_id << 32 | 0x4600 | vector);
	}


}

IOAPIC::IOAPIC(AdvancedConfigurationAndPowerInterface* acpi)
: m_acpi(acpi)
{

	// Get the information about the IO APIC
	m_madt = (MADT*) m_acpi->find("APIC");
	MADT_Item* io_apic_item = get_madt_item(MADT_TYPES::IO_APIC, 0);

	// Get the IO APIC
	auto* io_apic = (MADT_IO_APIC*) PhysicalMemoryManager::to_io_region((uint64_t) io_apic_item + sizeof(MADT_Item));
	PhysicalMemoryManager::s_current_manager->map((physical_address_t*) io_apic_item, (virtual_address_t*) (io_apic - sizeof(MADT_Item)), Present | Write);

	// Map the IO APIC address to the higher half
	m_address = io_apic->io_apic_address;
	m_address_high = (uint64_t) PhysicalMemoryManager::to_io_region(m_address);
	PhysicalMemoryManager::s_current_manager->map((physical_address_t*) m_address, (virtual_address_t*) m_address_high, Present | Write);
	Logger::DEBUG() << "IO APIC Address: phy=0x" << m_address << ", virt=0x" << m_address_high << "\n";

	// Get the IO APIC version and max redirection entry
	m_version = read(0x1);
	m_max_redirect_entry = (uint8_t) (m_version >> 16);

	// Log the IO APIC information
	Logger::DEBUG() << "IO APIC Version: 0x" << (uint64_t) (m_version & 0xFF) << "\n";
	Logger::DEBUG() << "IO APIC Max Redirection Entry: 0x" << (uint64_t) m_max_redirect_entry << "\n";

	// Get the source override item
	MADT_Item* source_override_item = get_madt_item(2, m_override_array_size);
	uint32_t total_length = sizeof(MADT);
	while (total_length < m_madt->header.length && m_override_array_size < 0x10) { // 0x10 is the max items

		total_length += source_override_item->length;

		// If there is an override, populate the array
		if (source_override_item != nullptr && source_override_item->type == 2) {

			// Get the override and populate the array
			auto* override = (Override*) (source_override_item + 1);
			m_override_array[m_override_array_size].bus = override->bus;
			m_override_array[m_override_array_size].source = override->source;
			m_override_array[m_override_array_size].global_system_interrupt = override->global_system_interrupt;
			m_override_array[m_override_array_size].flags = override->flags;
			m_override_array_size++;
		}

		// Get the next item
		source_override_item = get_madt_item(2, m_override_array_size);
		if (source_override_item == nullptr)
			break;
	}

	Logger::DEBUG() << "IO APIC Source Overrides: 0x" << m_override_array_size << "\n";
}

IOAPIC::~IOAPIC() = default;

/**
 * @brief Get an item in the MADT
 *
 * @param type The type of item
 * @param index The index of the item
 * @return The item or null if not found
 */
MADT_Item* IOAPIC::get_madt_item(uint8_t type, uint8_t index) {

	// The item starts at the start of the MADT
	auto* item = (MADT_Item*) ((uint64_t) m_madt + sizeof(MADT));
	uint64_t total_length = 0;
	uint8_t current_index = 0;

	// Loop through the items
	while (total_length + sizeof(MADT) < m_madt->header.length && current_index <= index) {

		// Correct type
		if (item->type == type) {

			// Correct index means found
			if (current_index == index)
				return item;

			// Right type wrong index so move on
			current_index++;
		}

		// Increment the total length
		total_length += item->length;
		item = (MADT_Item*) ((uint64_t) item + item->length);
	}

	// No item found
	return nullptr;
}

/**
 * @brief Read a value from a IO Apic register
 *
 * @param reg The register to read from
 * @return The value at the register
 */
uint32_t IOAPIC::read(uint32_t reg) const {

	// Tell the APIC what register to read from
	*(volatile uint32_t*) (m_address_high + 0x00) = reg;

	// Read the value
	return *(volatile uint32_t*) (m_address_high + 0x10);
}

/**
 * @brief Write a value to an IO Apic register
 *
 * @param reg The register to write to
 * @param value The value to set the register to
 * @return The value at the register
 */
void IOAPIC::write(uint32_t reg, uint32_t value) const {

	// Write the register
	*(volatile uint32_t*) (m_address_high + 0x00) = reg;

	// Write the value
	*(volatile uint32_t*) (m_address_high + 0x10) = value;
}

/**
 * @brief Read a redirect entry into a buffer
 *
 * @param index The index of the entry
 * @param entry The buffer to read into
 */
void IOAPIC::read_redirect(uint8_t index, RedirectionEntry* entry) {

	// Check bounds
	if (index < 0x10 || index > 0x3F)
		return;

	// Read the entry chunks into the buffer (struct is auto extracted from the raw information)
	uint32_t low = read(index);
	uint32_t high = read(index + 1);
	entry->raw = ((uint64_t) high << 32) | ((uint64_t) low);
}

/**
 * @brief Write a redirect entry from a buffer
 *
 * @param index The index of the entry
 * @param entry The buffer to write into
 */
void IOAPIC::write_redirect(uint8_t index, RedirectionEntry* entry) {

	// Check bounds
	if (index < 0x10 || index > 0x3F)
		return;

	// Break the entry down into 32bit chunks
	auto low = (uint32_t) entry->raw;
	auto high = (uint32_t) (entry->raw >> 32);

	// Set the entry
	write(index, low);
	write(index + 1, high);
}

/**
 * @brief Redirect a system interrupt to a different IRQ
 *
 * @param redirect The redirection entry
 */
void IOAPIC::set_redirect(interrupt_redirect_t* redirect) {

	// Create the redirection entry
	RedirectionEntry entry = {};
	entry.raw = redirect->flags | (redirect->interrupt & 0xFF);
	entry.destination = redirect->destination;
	entry.mask = redirect->mask;

	// Check if a global system interrupt is used
	for (uint8_t i = 0; i < m_override_array_size; i++) {

		if (m_override_array[i].source != redirect->type)
			continue;

		// Set the lower 4 bits of the pin
		entry.pin_polarity = ((m_override_array[i].flags & 0b11) == 2) ? 0b1 : 0b0;
		entry.pin_polarity = (((m_override_array[i].flags >> 2) & 0b11) == 2) ? 0b1 : 0b0;

		// Set the trigger mode
		entry.trigger_mode = (((m_override_array[i].flags >> 2) & 0b11) == 2);
		break;

	}

	// Write the redirect
	write_redirect(redirect->index, &entry);
}

/**
 * @brief Enables/Disables an interrupt redirect by masking it
 *
 * @param index The index of the redirect mask
 * @param mask True = masked = disabled, False = unmasked = enabled
 */
void IOAPIC::set_redirect_mask(uint8_t index, bool mask) {

	// Read the current entry
	RedirectionEntry entry = {};
	read_redirect(index, &entry);

	// Set the mask
	entry.mask = mask;
	write_redirect(index, &entry);
}

AdvancedProgrammableInterruptController::AdvancedProgrammableInterruptController(AdvancedConfigurationAndPowerInterface* acpi)
: m_pic_master_command_port(0x20),
  m_pic_master_data_port(0x21),
  m_pic_slave_command_port(0xA0),
  m_pic_slave_data_port(0xA1)
{

	// Register this APIC
	Logger::INFO() << "Setting up APIC\n";
	InterruptManager::active_interrupt_manager()->set_apic(this);

	// Init the Local APIC
	Logger::DEBUG() << "Initialising Local APIC\n";
	m_local_apic = new LocalAPIC();

	// Disable the legacy mode PIC
	Logger::DEBUG() << "Disabling PIC\n";
	disable_pic();

	// Init the IO APIC
	Logger::DEBUG() << "Initialising IO APIC\n";
	m_io_apic = new IOAPIC(acpi);
}

AdvancedProgrammableInterruptController::~AdvancedProgrammableInterruptController() {

	// Free the memory
	delete m_local_apic;
	delete m_io_apic;
}

/**
 * @brief Disable the legacy PIC to prevent it from sending interrupts
 */
void AdvancedProgrammableInterruptController::disable_pic() {

	// Initialise the PIC
	m_pic_master_command_port.write(0x11);
	m_pic_slave_command_port.write(0x11);

	// Set the offsets
	m_pic_master_data_port.write(0x20);
	m_pic_slave_data_port.write(0x28);

	// Set the slave/master relationships
	m_pic_master_data_port.write(0x04);
	m_pic_slave_data_port.write(0x02);

	// Set the modes (8086/8086)
	m_pic_master_data_port.write(0x01);
	m_pic_slave_data_port.write(0x01);

	// Mask the interrupts
	m_pic_master_data_port.write(0xFF);
	m_pic_slave_data_port.write(0xFF);
}

/**
 * @brief Get the local apic
 *
 * @return The local apic
 */
LocalAPIC* AdvancedProgrammableInterruptController::local_apic() const {

	return m_local_apic;
}

/**
 * @brief Get the io apic
 *
 * @return The io apic
 */
IOAPIC* AdvancedProgrammableInterruptController::io_apic() const {

	return m_io_apic;
}