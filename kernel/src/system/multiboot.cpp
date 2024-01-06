//
// Created by 98max on 1/6/2024.
//

#include <system/multiboot.h>
using namespace maxOS;
using namespace maxOS::system;
using namespace maxOS::common;

Multiboot::Multiboot(multiboot_info_t* boot_info, uint32_t magic)
: m_boot_info(boot_info),
  m_magic(magic)
{

}
Multiboot::~Multiboot() {

}

/**
 * @brief Returns the multiboot info structure
 *
 * @return The multiboot info structure
 */
multiboot_info_t* Multiboot::get_boot_info() {
  return m_boot_info;
}

/**
 * @brief Performs checks on the multiboot info structure
 *
 * @param error_stream The stream to write errors to
 * @return True if the checks passed, false otherwise
 */
bool Multiboot::check_flags(common::OutputStream *error_stream) {

  bool passed = true;

  // Check if the magic number is correct
  if (m_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        error_stream->write("ERROR: Invalid magic number: ");
        error_stream->write_hex(m_magic);
        error_stream->write("\n");
        passed = false;
  }
  error_stream->write(".");

  // Check if the memory pointers exist
  if (!check_flag(m_boot_info->flags, 0)) {
          error_stream->write("ERROR: Memory info not available\n");
          passed = false;
  }
  error_stream->write(".");

  // Check if the boot device exists
  if (!check_flag(m_boot_info->flags, 1)) {
          error_stream->write("ERROR: Boot device not available\n");
          passed = false;
  }
  error_stream->write(".");

  // FLAG 2: Skip command line check as it is not required
  // FLAG 3: Skip module check as it is not required

  // Check if both the a.out and ELF symbols are available (can only have one)
  if (check_flag(m_boot_info->flags, 4) && check_flag(m_boot_info->flags, 5)) {
          error_stream->write("ERROR: Both a.out and ELF symbols are available, should be mutually exclusive\n");
          passed = false;
  }
  error_stream->write(".");

  // Check if the a.out symbols are valid
  if (check_flag(m_boot_info->flags, 4)) {
      if (m_boot_info->u.aout_sym.tabsize == 0) {
              error_stream->write("ERROR: a.out symbol table is empty\n");
              passed = false;
      }
  }
  error_stream->write(".");

  // Check if the ELF symbols are valid
  if (check_flag(m_boot_info->flags, 5)) {
    if (m_boot_info->u.elf_sec.num == 0) {
            error_stream->write("ERROR: ELF section header table is empty\n");
            passed = false;
    }
  }error_stream->write(".");

  // Check if the memory map is valid
  if (!check_flag(m_boot_info->flags, 6)) {
          error_stream->write("ERROR: Memory map not available\n");
          passed = false;
  }
  error_stream->write(".");

  return passed;
}

/**
 * @brief Checks if a flag is set in the multiboot info structure
 *
 * @param flag The flag to check
 * @param bit The bit to check
 * @return True if the flag is set, false otherwise
 */
bool Multiboot::check_flag(uint32_t flag, uint32_t bit) {
    return (flag & (1 << bit));
}
