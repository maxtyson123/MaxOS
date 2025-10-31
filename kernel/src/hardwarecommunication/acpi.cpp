/**
 * @file acpi.cpp
 * @brief Implementation of the Advanced Configuration and Power Interface (ACPI) class
 *
 * @date 26th February 2024
 * @author Max Tyson
 */
#include <hardwarecommunication/acpi.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::common;

/**
 * @brief Construct a new Advanced Configuration And Power Interface object. Maps the ACPI headers and tables into the higher half.
 *
 * @param multiboot The multiboot information structure to get the ACPI information from
 */
AdvancedConfigurationAndPowerInterface::AdvancedConfigurationAndPowerInterface(system::Multiboot* multiboot) {

	Logger::INFO() << "Setting up ACPI\n";

	// If the new ACPI is not supported, panic
	ASSERT(multiboot->new_acpi() != nullptr || multiboot->old_acpi() != nullptr, "No ACPI found!");

	// Check if the new ACPI is supported
	m_using_new_acpi = multiboot->old_acpi() == nullptr;
	Logger::DEBUG() << "CPU Supports " << (m_using_new_acpi ? "New" : "Old") << " ACPI\n";

	if (m_using_new_acpi)
		m_rsdp2 = (RSDPDescriptor2*) (multiboot->new_acpi() + 1);
	else
		m_rsdp = (RSDPDescriptor*) (multiboot->old_acpi() + 1);

	// Map the XSDT/RSDT
	uint64_t physical_address = m_using_new_acpi ? m_rsdp2->xsdt_address : m_rsdp->rsdt_address;
	void* virtual_address = map_descriptor(physical_address);
	ASSERT(virtual_address != nullptr, "Failed to map ACPI table");
	Logger::DEBUG() << "XSDT/RSDT: physical: 0x" << physical_address << ", virtual: 0x" << (uint64_t)virtual_address << "\n";

	// Load
	if (m_using_new_acpi)
		m_xsdt = (XSDT*)virtual_address;
	else
		m_rsdt = (RSDT*)virtual_address;

	// Map the Tables
	Logger::DEBUG() << "Mapping ACPI Tables\n";
	map_tables(m_using_new_acpi ? sizeof(uint64_t) : sizeof(uint32_t));

	// Check if the checksum is valid
	ASSERT(valid_checksum(), "ACPI: Invalid checksum!");
}

AdvancedConfigurationAndPowerInterface::~AdvancedConfigurationAndPowerInterface() = default;

/**
 * @brief Maps the tables into the higher half
 *
 * @param pointer_size The size of the tables
 */
void AdvancedConfigurationAndPowerInterface::map_tables(uint8_t pointer_size) {

	size_t entries = (m_header->length - sizeof(ACPISDTHeader)) / pointer_size;
	for (uint32_t i = 0; i < entries; i++) {

		// Get the address (aligned to page)
		auto address = (uint64_t) (m_using_new_acpi ? m_xsdt->pointers[i] : get_rsdt_pointer(i));
		address = PhysicalMemoryManager::align_direct_to_page((size_t) address);

		// Map to the higher half
		PhysicalMemoryManager::s_current_manager->map((physical_address_t*) address, (void*) PhysicalMemoryManager::to_io_region(address), PRESENT | WRITE);

		// Reserve the memory
		PhysicalMemoryManager::s_current_manager->reserve(address);
	}

}

virtual_address_t* AdvancedConfigurationAndPowerInterface::map_descriptor(uint64_t physical_address) {

	// Get the base page
	uint64_t page = PhysicalMemoryManager::align_direct_to_page(physical_address);
	uint64_t offset = physical_address - page;

	// Map that page
	virtual_address_t* virtual_address = PhysicalMemoryManager::to_io_region(page);
	PhysicalMemoryManager::s_current_manager -> map((physical_address_t*)page, virtual_address, PRESENT | WRITE);
	PhysicalMemoryManager::s_current_manager -> reserve(page);

	// Read the length
	m_header = (ACPISDTHeader*)((uint8_t*)virtual_address + offset);
	ASSERT(m_header->length >= sizeof(ACPISDTHeader), "ACPI table too short");

	// Where to stop looping
	uint64_t end = page + m_header->length - 1;
	end = PhysicalMemoryManager::align_direct_to_page(end);

	// Map the remaining pages
	for (uint64_t page_i = page + PAGE_SIZE; page_i <= end; page_i += PAGE_SIZE) {
		virtual_address_t* virtual_i = PhysicalMemoryManager::to_io_region(page_i);

		PhysicalMemoryManager::s_current_manager -> map((physical_address_t*)page_i, virtual_i, PRESENT | WRITE);
		PhysicalMemoryManager::s_current_manager->reserve(page_i);
	}

	return (void*) ((uint8_t*) PhysicalMemoryManager::to_io_region(page) + offset);
}

/**
 * @brief Gets a pointer from the RSDT
 *
 * @param index The index of the pointer to get
 * @return The pointer at the given index
 *
 * @todo UBSan issue: type mismatch so using memcpy as a workaround
 */
uint64_t AdvancedConfigurationAndPowerInterface::get_rsdt_pointer(size_t index) {
	uint8_t* raw = (uint8_t*) m_rsdt;
	size_t offset = sizeof(ACPISDTHeader) + index * sizeof(uint32_t);

	// Ensure offset is within mapped header length (optional but safe)
	ASSERT(offset + sizeof(uint32_t) <= m_header->length, "RSDT index out of bounds");

	// Read the pointer manually
	uint32_t value;
	memcpy(&value, raw + offset, sizeof(uint32_t));
	return value;
}

/**
 * @brief Validates the checksum of a descriptor
 *
 * @param descriptor The descriptor to validate
 * @param length The length of the descriptor
 * @return True if the checksum is valid, false otherwise
 */
bool AdvancedConfigurationAndPowerInterface::validate(const char* descriptor, size_t length) {

	// Checksum
	uint32_t sum = 0;

	// Calculate the checksum
	for (uint32_t i = 0; i < length; i++)
		sum += ((char*) descriptor)[i];

	// Check if the checksum is valid
	return ((sum & 0xFF) == 0);
}

/**
 * @brief Finds a table with the given signature
 *
 * @param signature The signature to search for
 * @return The table with the given signature, or nullptr if not found
 */
ACPISDTHeader* AdvancedConfigurationAndPowerInterface::find(char const* signature) {

	// Get the number of entries
	size_t entries = (m_header->length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);
	if (m_using_new_acpi) entries = (m_header->length - sizeof(ACPISDTHeader)) / sizeof(uint64_t);

	// Loop through all the entries
	for (size_t i = 0; i < entries; ++i) {

		// Get the entry
		auto* header = (ACPISDTHeader*) (m_using_new_acpi ? m_xsdt->pointers[i] : get_rsdt_pointer(i));

		// Move the header to the higher half
		header = (ACPISDTHeader*) PhysicalMemoryManager::to_io_region((uint64_t) header);

		// Check if the signature matches
		if (strncmp(header->signature, signature, 4))
			return header;
	}

	// Return null if no entry was found
	return nullptr;
}

/**
 * @brief Checks if the checksum is valid
 *
 * @return True if the checksum is valid
 */
bool AdvancedConfigurationAndPowerInterface::valid_checksum() {

	// Get the information about the ACPI
	char* check = m_using_new_acpi ? (char*) m_rsdp2 : (char*) m_rsdp;
	uint32_t length = m_using_new_acpi ? sizeof(RSDPDescriptor2) : sizeof(RSDPDescriptor);

	// Calculate the checksum
	uint8_t sum = 0;
	for (uint32_t i = 0; i < length; i++)
		sum += check[i];

	return sum == 0;
}