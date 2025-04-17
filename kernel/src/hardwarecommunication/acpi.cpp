//
// Created by 98max on 18/01/2024.
//
#include <hardwarecommunication/acpi.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::common;
AdvancedConfigurationAndPowerInterface::AdvancedConfigurationAndPowerInterface(system::Multiboot* multiboot) {

  Logger::INFO() << "Setting up ACPI\n";

  // If the new ACPI is not supported, panic
  ASSERT(multiboot->new_acpi() != nullptr || multiboot->old_acpi() != nullptr, "No ACPI found!");

  // Check if the new ACPI is supported
  m_using_new_acpi = multiboot->old_acpi() == nullptr;
  Logger::DEBUG() << "CPU Supports " << (m_using_new_acpi ? "New" : "Old") << " ACPI\n";


  if(m_using_new_acpi){

    // Get the RSDP & XSDT
    m_rsdp2 = (RSDPDescriptor2*)(multiboot->new_acpi() + 1);
    m_xsdt = (XSDT*) PhysicalMemoryManager::to_higher_region((uint64_t)m_rsdp2->xsdt_address);
  }else{

    // Get the RSDP & RSDT
    m_rsdp = (RSDPDescriptor*)(multiboot->old_acpi() + 1);
    m_rsdt = (RSDT*) PhysicalMemoryManager::to_higher_region((uint64_t)m_rsdp->rsdt_address);
  }

   // Map the XSDT/RSDT
  uint64_t physical_address = m_using_new_acpi ? m_rsdp2->xsdt_address : m_rsdp->rsdt_address;
  auto virtual_address = (uint64_t)PhysicalMemoryManager::to_higher_region(physical_address);
  PhysicalMemoryManager::s_current_manager->map((physical_address_t*)PhysicalMemoryManager::align_direct_to_page(physical_address), (virtual_address_t*)virtual_address, Present | Write);
  Logger::DEBUG() << "XSDT/RSDT: physical: 0x" << physical_address << ", virtual: 0x" << virtual_address << "\n";

  // Reserve the XSDT/RSDT
  PhysicalMemoryManager::s_current_manager->reserve(m_using_new_acpi ? (uint64_t)m_rsdp2->xsdt_address : (uint64_t)m_rsdp->rsdt_address);

  // Load the header
  m_header = m_using_new_acpi ? &m_xsdt->header : &m_rsdt->header;

  // Map the Tables
  Logger::DEBUG() << "Mapping ACPI Tables\n";
  map_tables(m_using_new_acpi ? sizeof (uint64_t) : sizeof (uint32_t));

  // Check if the checksum is valid
  ASSERT(valid_checksum(), "ACPI: Invalid checksum!");
}

AdvancedConfigurationAndPowerInterface::~AdvancedConfigurationAndPowerInterface() = default;


/**
 * @brief Maps the tables into the higher half
 *
 * @param size_of_header The size of the tables
 */
void AdvancedConfigurationAndPowerInterface::map_tables(uint8_t size_of_tables) {

  for(uint32_t i = 0; i < (m_header->length - sizeof(ACPISDTHeader)) / size_of_tables; i++) {

    // Get the address (aligned to page)
    auto address = (uint64_t) (m_using_new_acpi ? m_xsdt->pointers[i] : m_rsdt->pointers[i]);
    address = PhysicalMemoryManager::align_direct_to_page((size_t)address);

    // Map to the higher half
    PhysicalMemoryManager::s_current_manager->map((physical_address_t*)address, (void*)PhysicalMemoryManager::to_io_region(address), Present | Write);

    // Reserve the memory
    PhysicalMemoryManager::s_current_manager->reserve(address);
  }

}


bool AdvancedConfigurationAndPowerInterface::validate(const char*descriptor, size_t length) {
  // Checksum
  uint32_t sum = 0;

  // Calculate the checksum
  for(uint32_t i = 0; i < length; i++)
        sum += ((char*)descriptor)[i];

  // Check if the checksum is valid
  return ((sum & 0xFF) == 0);

}



ACPISDTHeader* AdvancedConfigurationAndPowerInterface::find(char const *signature) {


  // Get the number of entries
  size_t entries = (m_header->length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);
  if(m_using_new_acpi) entries = (m_header->length - sizeof(ACPISDTHeader)) / sizeof(uint64_t);

  // Loop through all the entries
  for (size_t i = 0; i < entries; ++i) {

      // Get the entry
      auto* header = (ACPISDTHeader*) (m_using_new_acpi ? m_xsdt->pointers[i] : m_rsdt->pointers[i]);

      // Move the header to the higher half
      header = (ACPISDTHeader*)PhysicalMemoryManager::to_io_region((uint64_t)header);

      // Check if the signature matches
      if(strncmp(header->signature, signature, 4) != 0)
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
  char* check = m_using_new_acpi ? (char*)m_rsdp2 : (char*)m_rsdp;
  uint32_t length = m_using_new_acpi ? sizeof(RSDPDescriptor2) : sizeof(RSDPDescriptor);

  // Calculate the checksum
  uint8_t sum = 0;
  for(uint32_t i = 0; i < length; i++)
    sum += check[i];

  return sum == 0;

}
