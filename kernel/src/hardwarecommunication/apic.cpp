//
// Created by 98max on 18/01/2024.
//
#include <hardwarecommunication/apic.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::memory;

LocalAPIC::LocalAPIC() {

}

LocalAPIC::~LocalAPIC() {

}

void LocalAPIC::init() {

  // Read information about the local APIC
  uint64_t msr_info = CPU::read_msr(0x1B);

  // Get the APIC base address
  m_apic_base = msr_info & 0xFFFFF000;

  // Read if the APIC supports x2APIC
  uint32_t ignored, xleaf, x2leaf;
  CPU::cpuid(0x01, &ignored, &ignored, &x2leaf, &xleaf);

  if(x2leaf & (1 << 21)) {

    // Enable x2APIC
    m_x2apic = true;
    msr_info |= (1 << 10);
    CPU::write_msr(0x1B, msr_info);
    _kprintf("CPU supports x2APIC\n");

  } else if (xleaf & (1 << 9)) {

    m_x2apic = false;
    _kprintf("CPU supports xAPIC\n");

    // Map the APIC base address to the higher half
    m_apic_base_high = (uint64_t)MemoryManager::to_io_region(m_apic_base);
    PhysicalMemoryManager::s_current_manager->map((physical_address_t*)m_apic_base, (virtual_address_t*)m_apic_base_high, Write | Present);
    _kprintf("APIC Base: phy=0x%x, virt=0x%x\n", m_apic_base, m_apic_base_high);

  } else {
    ASSERT(false, "CPU does not support xAPIC");
  }

  // Get information about the APIC
  uint32_t spurious_vector = read(0xF0);
  bool is_enabled = msr_info & (1 << 11);
  bool is_bsp = msr_info & (1 << 8);
  _kprintf("APIC: boot processor: %d, enabled (globally): %d, spurious vector: 0x%x\n", is_bsp, is_enabled, spurious_vector);

  if(!is_enabled) {
    _kprintf("APIC is not enabled\n");
    return;
  }

  // Enable the APIC
  write(0xF0, (1 << 8) | 0x100);
  _kprintf("APIC Enabled\n");

  // Reserve the APIC base
  PhysicalMemoryManager::s_current_manager->reserve(m_apic_base);

  // Read the APIC version
  uint32_t version = read(0x30);
  _kprintf("APIC Version: 0x%x\n", version & 0xFF);

}

uint32_t LocalAPIC::read(uint32_t reg) {

  // If x2APIC is enabled, use the x2APIC MSR
  if(m_x2apic) {
      return (uint32_t)CPU::read_msr((reg >> 4) + 0x800);
  } else {
      return (*(volatile uint32_t*)((uintptr_t)m_apic_base_high + reg));

  }

}

void LocalAPIC::write(uint32_t reg, uint32_t value) {

  // If x2APIC is enabled, use the x2APIC MSR
  if(m_x2apic) {
      CPU::write_msr((reg >> 4) + 0x800, value);
  } else {
      (*(volatile uint32_t*)((uintptr_t)m_apic_base_high + reg)) = value;
    }
}

uint32_t LocalAPIC::id() {

  // Read the id
  uint32_t id = read(0x20);

  // Return the id
  return m_x2apic ? id : (id >> 24);

}

void LocalAPIC::send_eoi() {

    // Send the EOI
    write(0xB0, 0);
}

IOAPIC::IOAPIC(AdvancedConfigurationAndPowerInterface* acpi)
: m_acpi(acpi),
  m_madt(nullptr)
{

}

IOAPIC::~IOAPIC() {

}

void IOAPIC::init() {

  // Get the information about the IO APIC
  m_madt = (MADT*)m_acpi->find("APIC");
  MADT_Item* io_apic_item = get_madt_item(1, 0);

  // Get the IO APIC
  MADT_IOAPIC* io_apic = (MADT_IOAPIC*)MemoryManager::to_io_region((uint64_t)io_apic_item + sizeof(MADT_Item));
  PhysicalMemoryManager::s_current_manager->map((physical_address_t*)io_apic_item, (virtual_address_t*)(io_apic - sizeof(MADT_Item)), Present | Write);


  // Map the IO APIC address to the higher half
  m_address = io_apic->io_apic_address;
  m_address_high = (uint64_t)MemoryManager::to_io_region(m_address);
  PhysicalMemoryManager::s_current_manager->map((physical_address_t*)m_address, (virtual_address_t*)m_address_high, Present | Write);
  _kprintf("IO APIC Address: phy=0x%x, virt=0x%x\n", m_address, m_address_high);

  // Get the IO APIC version and max redirection entry
  m_version = read(0x1);
  m_max_redirect_entry = (uint8_t)(m_version >> 16);

  // Log the IO APIC information
  _kprintf("IO APIC Version: 0x%x\n", m_version);
  _kprintf("IO APIC Max Redirection Entry: 0x%x\n", m_max_redirect_entry);

  // Get the source override item
  MADT_Item* source_override_item = get_madt_item(2, m_override_array_size);

  // Loop through the source override items
  uint32_t total_length = sizeof(MADT);
  while (total_length < m_madt->header.length && m_override_array_size < 0x10){ // 0x10 is the max items

      // Increment the total length
      total_length += source_override_item->length;

      // If there is an override, populate the array
      if(source_override_item != nullptr && source_override_item->type == 2) {

          // Get the override and populate the array
          Override *override = (Override *)(source_override_item + 1);
          m_override_array[m_override_array_size].bus = override->bus;
          m_override_array[m_override_array_size].source = override->source;
          m_override_array[m_override_array_size].global_system_interrupt = override->global_system_interrupt;
          m_override_array[m_override_array_size].flags = override->flags;

          // Increment the override array size
          m_override_array_size++;
      }

      // Get the next item
      source_override_item = get_madt_item(2, m_override_array_size);

      // If there is no next item then break
      if(source_override_item == nullptr)
          break;
  }

  // Log how many overrides were found
  _kprintf("IO APIC Source Overrides: 0x%x\n", m_override_array_size);
}

MADT_Item *IOAPIC::get_madt_item(uint8_t type, uint8_t index) {

    // The item starts at the start of the MADT
    MADT_Item* item = (MADT_Item*)((uint64_t)m_madt + sizeof(MADT));
    uint64_t total_length = 0;
    uint8_t current_index = 0;

    // Loop through the items
    while (total_length + sizeof(MADT) < m_madt->header.length && current_index <= index) {

        // Check if the item is the correct type
        if(item->type == type) {

            // Check if the item is the correct index
            if(current_index == index) {
                return item;
            }

            // Increment the index
            current_index++;
        }

        // Increment the total length
        total_length += item->length;

        // Increment the item
        item = (MADT_Item*)((uint64_t)item + item->length);
    }

    // Return null if the item was not found
    return nullptr;
}

uint32_t IOAPIC::read(uint32_t reg) {

  // Write the register
  *(volatile uint32_t*)(m_address_high + 0x00) = reg;

  // Return the value
  return *(volatile uint32_t*)(m_address_high + 0x10);



}

void IOAPIC::write(uint32_t reg, uint32_t value) {

    // Write the register
    *(volatile uint32_t*)(m_address_high + 0x00) = reg;

    // Write the value
    *(volatile uint32_t*)(m_address_high + 0x10) = value;
}

void IOAPIC::read_redirect(uint8_t index, RedirectionEntry *entry) {

  // If the index is out of bounds, return
  if(index < 0x10 || index > 0x3F)
    return;

  // Low and high registers
  uint32_t low = read(index);
  uint32_t high = read(index + 1);

  // Set the entry
  entry->raw = ((uint64_t)high << 32) | ((uint64_t)low);

}

void IOAPIC::write_redirect(uint8_t index, RedirectionEntry *entry) {

  // If the index is out of bounds, return
  if(index < 0x10 || index > 0x3F)
    return;

  // Low and high registers
  uint32_t low = (uint32_t)entry->raw;
  uint32_t high = (uint32_t)(entry->raw >> 32);

  // Set the entry
  write(index, low);
  write(index + 1, high);
}

void IOAPIC::set_redirect(interrupt_redirect_t *redirect) {

    // Create the redirection entry
    RedirectionEntry entry;
    entry.raw = redirect->flags | (redirect -> interrupt & 0xFF);
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
void IOAPIC::set_redirect_mask(uint8_t index, bool mask) {

    // Read the current entry
    RedirectionEntry entry;
    read_redirect(index, &entry);

    // Set the mask
    entry.mask = mask;

    // Write the entry
    write_redirect(index, &entry);
}

AdvancedProgrammableInterruptController::AdvancedProgrammableInterruptController(AdvancedConfigurationAndPowerInterface* acpi)
: m_local_apic(),
  m_io_apic(acpi),
  m_pic_master_command_port(0x20),
  m_pic_master_data_port(0x21),
  m_pic_slave_command_port(0xA0),
  m_pic_slave_data_port(0xA1)
{

  // Init the Local APIC
  _kprintf("Initialising Local APIC\n");
  m_local_apic.init();

  // Disable the old PIC
  _kprintf("Disabling PIC\n");
  disable_pic();

  // Init the IO APIC
  _kprintf("Initialising IO APIC\n");
  m_io_apic.init();

}

AdvancedProgrammableInterruptController::~AdvancedProgrammableInterruptController() {

}

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
LocalAPIC *AdvancedProgrammableInterruptController::get_local_apic() {
  return &m_local_apic;
}
IOAPIC *AdvancedProgrammableInterruptController::get_io_apic() {
    return &m_io_apic;
}