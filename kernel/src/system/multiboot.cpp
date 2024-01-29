//
// Created by 98max on 1/6/2024.
//

#include <system/multiboot.h>
using namespace MaxOS;
using namespace MaxOS::system;

Multiboot::Multiboot(unsigned long addr) {


    // Loop through the tags and load them
    struct multiboot_tag *tag;
    for(tag=(struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {

      switch (tag -> type) {
          case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
              m_framebuffer = (multiboot_tag_framebuffer *)tag;
              break;

          case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
              m_basic_meminfo = (multiboot_tag_basic_meminfo *)tag;
              break;

          case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
              m_bootloader_name = (multiboot_tag_string *)tag;
              break;

          case MULTIBOOT_TAG_TYPE_MMAP:
                m_mmap = (multiboot_tag_mmap *)tag;
                break;

          case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                m_old_acpi = (multiboot_tag_old_acpi *)tag;
                break;

          case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                m_new_acpi = (multiboot_tag_new_acpi *)tag;
                break;
      }
    }
}

Multiboot::~Multiboot() {

}


multiboot_tag_framebuffer *Multiboot::get_framebuffer() {

    return m_framebuffer;

}

multiboot_tag_basic_meminfo *Multiboot::get_basic_meminfo() {

    return m_basic_meminfo;

}

multiboot_tag_string *Multiboot::get_bootloader_name() {

    return m_bootloader_name;

}

multiboot_tag_mmap *Multiboot::get_mmap() {

    return m_mmap;

}

multiboot_tag_old_acpi *Multiboot::get_old_acpi() {

  return m_old_acpi;
}


multiboot_tag_new_acpi *Multiboot::get_new_acpi() {

  return m_new_acpi;
}
