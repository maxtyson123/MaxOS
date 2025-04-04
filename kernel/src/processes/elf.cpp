//
// Created by 98max on 21/03/2025.
//

#include <processes/elf.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::memory;

char Elf64::elf_magic[4] = {0x7F, 'E', 'L', 'F'};

/**
 * @brief Constructor for the Elf64 class
 *
 * @param elf_header_address The address of the elf header, this must be mapped to memory before uses
 */
Elf64::Elf64(uintptr_t elf_header_address)
: m_elf_header_address(elf_header_address)
{
}

/**
 * @brief Destructor for the Elf64 class
 */
Elf64::~Elf64()
= default;

/**
 * @brief Loads the elf program into memory if a valid elf file
 */
void Elf64::load() {

  // Check if valid
  if(!is_valid()) return; //TODO: error handling when the syscall for this is implemented

  // Load the program headers
  load_program_headers();

}

/**
 * @brief Gets the header of the elf file
 *
 * @return The header of the elf file
 */
elf_64_header_t *Elf64::header() const {

  // Return the header
  return (elf_64_header_t*)m_elf_header_address;
}

/**
 * @brief Gets a program header from the elf file
 *
 * @param index The index of the program header
 * @return The program header at that index or nullptr if out of bounds
 */
elf_64_program_header_t* Elf64::get_program_header(size_t index) {

  // Check if within bounds
  if(index >= header() -> program_header_count) return nullptr;

  // Get the address of the program headers
  auto* program_headers = (elf_64_program_header_t*)(m_elf_header_address + header() -> program_header_offset);

  // Return the requested program header
  return &program_headers[index];

}

/**
 * @brief Gets a section header from the elf file
 *
 * @param index The index of the section header
 * @return The section header at that index or nullptr if out of bounds
 */
elf_64_section_header_t *Elf64::get_section_header(size_t index) {

  // Check if within bounds
  if(index >= header() -> section_header_count) return nullptr;

  // Get the address of the section headers
  auto* section_headers = (elf_64_section_header_t*)(m_elf_header_address + header() -> section_header_offset);

  // Return the requested section header
  return &section_headers[index];

}

/**
 * @brief Checks if the elf file is valid for MaxOS runtime
 */
bool Elf64::is_valid() {

  // Validate the magic number
  for (size_t i = 0; i < 4; i++)
    if (header() -> identification[i] != elf_magic[i])
      return false;

  // Check if the elf is 64 bit
  if(header() -> identification[(int)ElfIdentification::Class] != (int)ElfClass::Bits64)
    return false;

  // Check if the elf is little endian
  if(header() -> identification[(int)ElfIdentification::Data] != (int)ElfData::LittleEndian)
    return false;

  // Check if the elf is version 1
  if(header() -> identification[(int)ElfIdentification::Version] != (int)ElfVersion::Current)
    return false;

  // Check if the elf is for the MaxOS platform
  //  if(header() -> identification[OSABI] != MaxOSABI)
  //      return false; TODO: Would be nice to have an OSABI

  // Check if the elf is executable
  if(header() -> type != (int)ElfType::Executable)
    return false;

  // Check if the elf is for the x86_64 platform
  if(header() -> machine != (int)ElfMachine::x86_64)
    return false;

  // LGTM
  return true;

}

/**
 * @brief Loop through the program headers and load the program into memory at the give address with the given size
 */
void Elf64::load_program_headers() {

    // Loop through the program headers
    for (size_t i = 0; i < header() -> program_header_count; i++) {

        // Get the program header
        elf_64_program_header_t* program_header = get_program_header(i);

        // Check if the program header is loadable
        if(program_header -> type != (int)ElfProgramType::Load)
          continue;

        // Type of the program header
        uint64_t flags = to_vmm_flags(program_header->flags);

        // Allocate space at the requested address
        void* address = MemoryManager::s_current_memory_manager -> vmm() -> allocate(program_header -> virtual_address, program_header -> memory_size, Present | PageFlags::Write);
        ASSERT(address != nullptr, "Failed to allocate memory for program header\n");

        // Copy the program to the address
        memcpy((void*)address, (void*)(m_elf_header_address + program_header -> offset), program_header -> file_size);

        // Zero the rest of the memory if needed
        size_t zero_size = program_header -> memory_size - program_header -> file_size;
        memset((void*)((uintptr_t)address + program_header -> file_size), 0, zero_size);

        // Now that we are done with modifying the memory, we should set the flags to the correct ones
        PhysicalMemoryManager::s_current_manager -> change_page_flags((virtual_address_t*)address, flags, MemoryManager::s_current_memory_manager -> vmm() -> pml4_root_address());
    }

}

/**
 * @brief Converts elf flags to vmm flags
 *
 * @param type The elf flags of the program header
 * @return The vmm flags
 */
uint64_t Elf64::to_vmm_flags(uint32_t type) {

  // Conversion
  // ELF   |   VMM
  // 0x0   |   Executable (not used)
  // 0x1   |   Write
  // 0x2   |   Read

  uint64_t flags = Present | User | NoExecute;

  // Enable write
  if(type & ElfWrite)
      flags |= Write;

  // Disable no execute
  if(type & ElfProgramFlags::ElfExecute)
     flags &= ~NoExecute;


  return flags;

}

