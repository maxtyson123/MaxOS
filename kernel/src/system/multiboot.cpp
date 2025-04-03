//
// Created by 98max on 1/6/2024.
//

#include <system/multiboot.h>
#include <common/kprint.h>
#include <memory/memorymanagement.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::memory;

Multiboot::Multiboot(unsigned long address, unsigned long magic)
: m_base_address(address)
{

  // Confirm the bootloader
  ASSERT(magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Multiboot2 Bootloader Not Detected");

    _kprintf("Multiboot\n");

    // Loop through the tags and load them
    for(multiboot_tag* tag = start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {

      switch (tag -> type) {
          case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
              m_framebuffer = (multiboot_tag_framebuffer *)tag;
              break;

          case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
              m_basic_meminfo = (multiboot_tag_basic_meminfo *)tag;
              break;

          case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
              m_bootloader_name = (multiboot_tag_string *)tag;
              _kprintf("Bootloader: %s\n", m_bootloader_name->string);
              break;

          case MULTIBOOT_TAG_TYPE_BOOTDEV:
            multiboot_tag_bootdev *bootdev;
            bootdev = (multiboot_tag_bootdev *)tag;
            _kprintf("Boot device: 0x%x, 0x%x, 0x%x of type 0x%x\n",
                    (unsigned) bootdev->biosdev, (unsigned) bootdev->slice,
                    (unsigned) bootdev->part, (unsigned) bootdev->type);
            break;

          case MULTIBOOT_TAG_TYPE_MMAP:

            // If there is not already a mmap tag, set it
            if (m_mmap == nullptr)
                m_mmap = (multiboot_tag_mmap *)tag;

            break;

          case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                m_old_acpi = (multiboot_tag_old_acpi *)tag;
                break;


          case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                m_new_acpi = (multiboot_tag_new_acpi *)tag;
                break;

          case MULTIBOOT_TAG_TYPE_MODULE:
              multiboot_tag_module *module;
              module = (multiboot_tag_module *)tag;
              _kprintf("Module: start=0x%x, end=0x%x, cmdline=%s\n",
                        (unsigned) module->mod_start,
                        (unsigned) module->mod_end,
                        module->cmdline);
              m_module = module;
              break;
          }
    }
}

Multiboot::~Multiboot() = default;


multiboot_tag_framebuffer *Multiboot::framebuffer() {

    return m_framebuffer;

}

multiboot_tag_basic_meminfo *Multiboot::basic_meminfo() {

    return m_basic_meminfo;

}

multiboot_tag_string *Multiboot::bootloader_name() {

    return m_bootloader_name;

}

multiboot_tag_mmap *Multiboot::mmap() {

    return m_mmap;

}

multiboot_tag_old_acpi *Multiboot::old_acpi() {

  return m_old_acpi;
}


multiboot_tag_new_acpi *Multiboot::new_acpi() {

  return m_new_acpi;
}

/**
 * @brief Check if an address is reserved by a multiboot module
 * @param address The address to check
 * @return True if the address is reserved
 */
bool Multiboot::is_reserved(multiboot_uint64_t address) {

  // Loop through the tags checking if the address is reserved
  for(multiboot_tag* tag = start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {

      // Check if the tag is a module or mmap
      if(tag -> type != MULTIBOOT_TAG_TYPE_MODULE && tag -> type != MULTIBOOT_TAG_TYPE_MMAP)
        continue;

      // Get the module tag
      auto* module = (struct multiboot_tag_module*)tag;

      // Check if the address is within the module
      if(address >= module -> mod_start && address < module -> mod_end)
        return true;
  }


  // Not part of multiboot
  return false;

}

/**
 * Get the start tag of the multiboot information (useful for iterating through the tags)
 *
 * @return The start tag
 */
multiboot_tag *Multiboot::start_tag() const {

  return (multiboot_tag*)(m_base_address + PhysicalMemoryManager::s_higher_half_kernel_offset + 8);
}
