//
// Created by 98max on 18/01/2024.
//
#include <hardwarecommunication/acpi.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::memory;
AdvancedConfigurationAndPowerInterface::AdvancedConfigurationAndPowerInterface(system::Multiboot* multiboot) {

  if(multiboot->get_old_acpi() != 0){

    // Get the RSDP & RSDT
    RSDPDescriptor* rsdp = (RSDPDescriptor*)(multiboot->get_old_acpi() + 1);
    m_rsdt = (RSDT*) rsdp->rsdt_address;

    // Map the RSDT
    PhysicalMemoryManager::current_manager->map(VirtualPointer(m_rsdt), VirtualPointer(MemoryManager::to_higher_region((uint64_t)m_rsdt)), PageFlags::Write);
    m_rsdt = (RSDT*) MemoryManager::to_higher_region((uint64_t)m_rsdt);

    // Load the header
    m_header = &m_rsdt->header;

    // Calculate the checksum
    uint8_t sum = 0;
    for(uint32_t i = 0; i < sizeof(RSDPDescriptor); i++)
              sum += ((char*)rsdp)[i];

    // Check if the checksum is valid
    ASSERT(sum == 0, "Invalid checksum!")

  }else{

    // If the new ACPI is not supported, panic
    ASSERT(multiboot->get_new_acpi() != 0, "No ACPI found!")

    // It's the new ACPI
    m_type = 1;

    // Get the RSDP & XSDT
    RSDPDescriptor2* rsdp2 = (RSDPDescriptor2*)(multiboot->get_new_acpi() + 1);
    m_xsdt = (XSDT*) rsdp2->xsdt_address;

    // Load the header
    m_header = &m_xsdt->header;

    // Calculate the checksum
    uint8_t sum = 0;
    for(uint32_t i = 0; i < sizeof(RSDPDescriptor2); i++)
        sum += ((char*)rsdp2)[i];

    // Check if the checksum is valid
    ASSERT(sum == 0, "Invalid checksum!")

  }
}

AdvancedConfigurationAndPowerInterface::~AdvancedConfigurationAndPowerInterface() {

}

bool AdvancedConfigurationAndPowerInterface::validate(const char* discriptor, size_t length) {
  // Checksum
  uint32_t sum = 0;

  // Calculate the checksum
  for(uint32_t i = 0; i < length; i++)
        sum += ((char*) discriptor)[i];

  // Check if the checksum is valid
  return ((sum & 0xFF) == 0);

}



ACPISDTHeader* AdvancedConfigurationAndPowerInterface::find(char const *signature) {


  // Get the number of entries
  size_t entries = (m_header->length - sizeof(ACPISDTHeader)) / 4;
  if(m_type) entries = (m_header->length - sizeof(ACPISDTHeader)) / 8;

  // Loop through all the entries
  for (size_t i = 0; i < entries; ++i) {

      // Get the entry
      ACPISDTHeader* header = (ACPISDTHeader*) (m_type ? m_xsdt->pointers[i] : m_rsdt->pointers[i]);

      // Check if the signature matches
      if(strncmp(header->signature, signature, 4) == 0)
         return header;
  }

  // Return null if no entry was found
  return nullptr;
}