//
// Created by 98max on 18/01/2024.
//
#include <hardwarecommunication/acpi.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
AdvancedConfigurationAndPowerInterface::AdvancedConfigurationAndPowerInterface(system::Multiboot* multiboot) {

  if(multiboot->get_old_acpi() != 0){

    // Get the RSDP & RSDT
    RSDPDescriptor* rsdp = (RSDPDescriptor*) multiboot->get_old_acpi();
    m_rsdt = (RSDT*) rsdp->rsdt_address;

    // Load the header
    m_header = &m_rsdt->header;

    // Check if the checksum is valid
    if(!validate((char*) m_rsdt, m_header->length))
      _kprintf("ACPI: Invalid checksum!\n");

  }else{

    // Its the new ACPI
    m_type = 1;

    // Get the RSDP & XSDT
    RSDPDescriptor2* rsdp = (RSDPDescriptor2*) multiboot->get_new_acpi();
    m_xsdt = (XSDT*) rsdp->xsdt_address;

    // Load the header
    m_header = &m_xsdt->header;

    // Check if the checksum is valid
    if(!validate((char*) m_xsdt, m_header->length))
      _kprintf("ACPI: Invalid checksum!\n");

  }
}

AdvancedConfigurationAndPowerInterface::~AdvancedConfigurationAndPowerInterface() {

}

bool AdvancedConfigurationAndPowerInterface::validate(char *discriptor, size_t length) {

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
  for (int i = 0; i < entries; ++i) {

      // Get the entry
      ACPISDTHeader* header = (ACPISDTHeader*) (m_type ? m_xsdt->pointers[i] : m_rsdt->pointers[i]);

      // Check if the signature matches
      if(strncmp(header->signature, signature, 4) == 0)
         return header;
  }
}