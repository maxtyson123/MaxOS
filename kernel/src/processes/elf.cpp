/**
 * @file elf.cpp
 * @brief Implementation of an Executable and Linkable Format (ELF) loader for 64-bit binaries
 *
 * @date 21st March 2025
 * @author Max Tyson
 */

#include <processes/elf.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::memory;



/**
 * @brief Constructor for the ELF64 class
 *
 * @param elf_header_address The address of the elf header, this must be mapped to memory before uses
 */
ELF64::ELF64(uintptr_t elf_header_address)
: m_elf_header_address(elf_header_address)
{
}

/**
 * @brief Destructor for the ELF64 class
 */
ELF64::~ELF64() = default;

/**
 * @brief Loads the elf program into memory if a valid elf file
 *
 * @todo Error handling
 */
void ELF64::load() {

	if (!is_valid())
		return;

	load_program_headers();

}

/**
 * @brief Gets the header of the elf file
 *
 * @return The header of the elf file
 */
elf_64_header_t* ELF64::header() const {

	return (elf_64_header_t*) m_elf_header_address;
}

/**
 * @brief Gets a program header from the elf file
 *
 * @param index The index of the program header
 * @return The program header at that index or nullptr if out of bounds
 */
elf_64_program_header_t* ELF64::get_program_header(size_t index) const {

	// Check bounds
	if (index >= header()->program_header_count)
		return nullptr;

	// Find the program headers and return the item
	auto* program_headers = (elf_64_program_header_t*) (m_elf_header_address + header()->program_header_offset);
	return &program_headers[index];

}

/**
 * @brief Gets a section header from the elf file
 *
 * @param index The index of the section header
 * @return The section header at that index or nullptr if out of bounds
 */
elf_64_section_header_t* ELF64::get_section_header(size_t index) const {

	// Check bound
	if (index >= header()->section_header_count)
		return nullptr;

	// Find the section headers and return the item
	auto* section_headers = (elf_64_section_header_t*) (m_elf_header_address + header()->section_header_offset);
	return &section_headers[index];

}

/**
 * @brief Checks if the elf file is valid for MaxOS runtime
 *
 * @todo Add support for maxOS ABI
 */
bool ELF64::is_valid() const {

	// Validate the magic number
	for (size_t i = 0; i < 4; i++)
		if (header()->identification[i] != ELF_MAGIC[i])
			return false;

	// Check if the elf is 64 bit
	if (header()->identification[(int) ELFIdentification::Class] != (int) ELFClass::Bits64)
		return false;

	// Check if the elf is little endian
	if (header()->identification[(int) ELFIdentification::Data] != (int) ELFData::LittleEndian)
		return false;

	// Check if the elf is version 1
	if (header()->identification[(int) ELFIdentification::Version] != (int) ELFVersion::Current)
		return false;

	// Check if the elf is for the MaxOS platform
	//  if(header() -> identification[OSABI] != MaxOSABI)
	//      return false;

	// Check if the elf is executable
	if (header()->type != (int) ELFType::Executable)
		return false;

	// Check if the elf is for the x86_64 platform
	if (header()->machine != (int) ELFMachine::x86_64)
		return false;

	// LGTM
	return true;

}

/**
 * @brief Loop through the program headers and load the program into memory at the give address with the given size
 */
void ELF64::load_program_headers() const {

	for (size_t i = 0; i < header()->program_header_count; i++) {

		// Get the header information
		elf_64_program_header_t* program_header = get_program_header(i);

		// Only load headers that actually need loading
		if (program_header->type != (int) ELFProgramType::Load)
			continue;

		// Allocate space at the requested address
		void* address = MemoryManager::s_current_memory_manager->vmm()->allocate(program_header->virtual_address, program_header->memory_size, PRESENT | WRITE);
		ASSERT(address != nullptr, "Failed to allocate memory for program header\n");

		// Copy the program into memory at that address
		memcpy(address, (void*) (m_elf_header_address + program_header->offset), program_header->file_size);

		// Zero the rest of the memory if needed
		size_t zero_size = program_header->memory_size - program_header->file_size;
		memset((void*) ((uintptr_t) address + program_header->file_size), 0, zero_size);

		// Once the memory has been copied can now mark the pages as read only etc
		uint64_t flags = to_vmm_flags(program_header->flags);
		PhysicalMemoryManager::s_current_manager->change_page_flags(address, flags, MemoryManager::s_current_memory_manager->vmm()->pml4_root_address());
	}
}

/**
 * @brief Converts elf flags to vmm flags
 *
 * @param type The elf flags of the program header
 * @return The vmm flags
 */
uint64_t ELF64::to_vmm_flags(uint32_t type) {

	// Conversion
	// ELF   |   VMM
	// 0x0   |   Executable
	// 0x1   |   Write
	// 0x2   |   Read

	uint64_t flags = PRESENT | USER | NO_EXECUTE;

	// Enable write
	if (type & ELFWrite)
		flags |= WRITE;

	// Disable no execute
	if (type & ELFExecute)
		flags &= ~NO_EXECUTE;

	return flags;
}