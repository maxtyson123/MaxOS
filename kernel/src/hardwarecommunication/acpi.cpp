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


    _kprintf("Using old ACPI\n");


    // Get the RSDP & RSDT
    RSDPDescriptor* rsdp = (RSDPDescriptor*)(multiboot->get_old_acpi() + 1);
    m_rsdt = (RSDT*) rsdp->rsdt_address;

    // Map the RSDT
    PhysicalMemoryManager::s_current_manager->map(m_rsdt, MemoryManager::to_io_region((uint64_t)m_rsdt), Present | Write);
    m_rsdt = (RSDT*) MemoryManager::to_higher_region((uint64_t)m_rsdt);
    _kprintf("RSDT: physical: 0x%x, virtual: 0x%x\n", rsdp->rsdt_address, m_rsdt);


    // Load the header
    m_header = &m_rsdt->header;
    if((m_header->length / PhysicalMemoryManager::s_page_size + 1) > 1) {
      ASSERT(false, "RSDT is too big, need to map more pages!")
    }


    // Calculate the checksum
    uint8_t sum = 0;
    for(uint32_t i = 0; i < sizeof(RSDPDescriptor); i++)
              sum += ((char*)rsdp)[i];

    // Check if the checksum is valid
    ASSERT(sum == 0, "Invalid checksum!")

  }else{

    // TODO: MAP THE MF
    ASSERT(false, "Not implemented!")

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

      // Move the header to the higher half
      header = (ACPISDTHeader*) MemoryManager::to_higher_region((uint64_t)header);

      // Check if the signature matches
      if(strncmp(header->signature, signature, 4) == 0)
         return header;
  }

  // Return null if no entry was found
  return nullptr;
}