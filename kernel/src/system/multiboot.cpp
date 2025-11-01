/**
 * @file multiboot.cpp
 * @brief Implementation of a Multiboot2 information structure parser
 *
 * @date 6th January 2024
 * @author Max Tyson
 */

#include <system/multiboot.h>
#include <common/logger.h>
#include <memory/memorymanagement.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::common;

/**
 * @brief Constructor for the Multiboot class. Parses the multiboot info struct and loads the tags
 *
 * @param address The address of the multiboot info struct
 * @param magic The magic number to confirm the bootloader
 */
Multiboot::Multiboot(unsigned long address, unsigned long magic)
: start_address(address)
{

	// Confirm the bootloader
	ASSERT(magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Multiboot2 Bootloader Not Detected");
	Logger::DEBUG() << "Multiboot2 Bootloader Detected at 0x" << (uint64_t) address << "\n";

	multiboot_tag* tag = start_tag();

	// Loop through the tags and load them
	while (true) {

		// Handle the tag
		switch (tag->type) {

			case MULTIBOOT_TAG_TYPE_END:
				end_address = (unsigned long) PhysicalMemoryManager::to_lower_region((uint64_t) tag);
				return;

			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
				m_framebuffer = (multiboot_tag_framebuffer*) tag;
				break;

			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
				m_basic_meminfo = (multiboot_tag_basic_meminfo*) tag;
				break;

			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				m_bootloader_name = (multiboot_tag_string*) tag;
				Logger::DEBUG() << "Bootloader: " << m_bootloader_name->string << "\n";
				break;

			case MULTIBOOT_TAG_TYPE_BOOTDEV:
				multiboot_tag_bootdev* bootdev;
				bootdev = (multiboot_tag_bootdev*) tag;
				Logger::DEBUG() << "Boot device: drive=0x" << (uint64_t) bootdev->biosdev << ", partition=0x" << (uint64_t) bootdev->part << "\n";
				break;

			case MULTIBOOT_TAG_TYPE_MMAP:

				// If there is not already a mmap tag, set it
				if (m_mmap == nullptr)
					m_mmap = (multiboot_tag_mmap*) tag;

				break;

			case MULTIBOOT_TAG_TYPE_ACPI_OLD:
				m_old_acpi = (multiboot_tag_old_acpi*) tag;
				break;


			case MULTIBOOT_TAG_TYPE_ACPI_NEW:
				m_new_acpi = (multiboot_tag_new_acpi*) tag;
				break;

			case MULTIBOOT_TAG_TYPE_MODULE:
				multiboot_tag_module* module;
				module = (multiboot_tag_module*) tag;
				Logger::DEBUG() << "Module: start=0x" << (uint64_t) module->mod_start << ", end=0x" << (uint64_t) module->mod_end << ", cmdline=" << module->cmdline << "\n";
				m_module = module;
				break;
		}

		// Move to the next tag
		tag = (struct multiboot_tag*) ((multiboot_uint8_t*) tag + ((tag->size + 7) & ~7));
	}
}

Multiboot::~Multiboot() = default;

/**
 * @brief Get the framebuffer tag
 * @return The framebuffer tag
 */
multiboot_tag_framebuffer* Multiboot::framebuffer() {

	return m_framebuffer;
}

/**
 * @brief Get the basic memory info tag
 * @return The basic memory info tag
 */
multiboot_tag_basic_meminfo* Multiboot::basic_meminfo() {

	return m_basic_meminfo;
}

/**
 * @brief Get the bootloader name tag
 * @return The bootloader name tag
 */
multiboot_tag_string* Multiboot::bootloader_name() {

	return m_bootloader_name;
}

/**
 * @brief Get the module tag
 * @return The module tag
 */
multiboot_tag_mmap* Multiboot::mmap() {

	return m_mmap;
}

/**
 * @brief Get the old ACPI tag
 * @return The old ACPI tag
 */
multiboot_tag_old_acpi* Multiboot::old_acpi() {

	return m_old_acpi;
}

/**
 * @brief Get the new ACPI tag
 * @return The new ACPI tag
 */
multiboot_tag_new_acpi* Multiboot::new_acpi() {

	return m_new_acpi;
}

/**
 * @brief Check if an address is reserved by a multiboot module
 * @param address The address to check
 * @return True if the address is reserved
 */
bool Multiboot::is_reserved(multiboot_uint64_t address) {

	// Loop through the tags checking if the address is reserved
	for (multiboot_tag* tag = start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag*) ((multiboot_uint8_t*) tag + ((tag->size + 7) & ~7))) {

		// Check if the tag is a module or mmap
		if (tag->type != MULTIBOOT_TAG_TYPE_MODULE && tag->type != MULTIBOOT_TAG_TYPE_MMAP)
			continue;

		// Get the module tag
		auto* module = (struct multiboot_tag_module*) tag;

		// Check if the address is within the module
		if (address >= module->mod_start && address < module->mod_end)
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
multiboot_tag* Multiboot::start_tag() const {

	return (multiboot_tag*) (start_address + HIGHER_HALF_KERNEL_OFFSET + 8);
}
