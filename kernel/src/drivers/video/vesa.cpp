//
// Created by 98max on 19/07/2023.
//

#include <drivers/video/vesa.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::video;
using namespace MaxOS::memory;
using namespace MaxOS::system;
using namespace MaxOS::common;

VideoElectronicsStandardsAssociation::VideoElectronicsStandardsAssociation(multiboot_tag_framebuffer *framebuffer_info)
: m_framebuffer_info(framebuffer_info)
{

	Logger::INFO() << "Setting up VESA driver\n";

	// Save the framebuffer info
	m_bpp = m_framebuffer_info->common.framebuffer_bpp;
	m_pitch = m_framebuffer_info->common.framebuffer_pitch;
	m_framebuffer_size = m_framebuffer_info->common.framebuffer_height * m_pitch;
	this->set_mode(framebuffer_info->common.framebuffer_width, framebuffer_info->common.framebuffer_height,
				   framebuffer_info->common.framebuffer_bpp);
	Logger::DEBUG() << "Framebuffer: bpp=" << m_bpp << ", pitch=" << m_pitch << ", size=" << m_framebuffer_size << "\n";

	// Map the frame buffer into the higher half
	auto physical_address = (uint64_t) m_framebuffer_info->common.framebuffer_addr;
	m_framebuffer_address = (uint64_t *) PhysicalMemoryManager::to_dm_region(physical_address);
	PhysicalMemoryManager::s_current_manager->map_area((physical_address_t *) physical_address, m_framebuffer_address, m_framebuffer_size, Write | Present);

	// Reserve the physical memory
	size_t pages = PhysicalMemoryManager::size_to_frames(m_framebuffer_size);
	PhysicalMemoryManager::s_current_manager->reserve(m_framebuffer_info->common.framebuffer_addr, pages);

	// Log info
	Logger::DEBUG() << "Framebuffer address: physical=0x" << (uint64_t) physical_address << ", virtual=0x" << (uint64_t) m_framebuffer_address << "\n";
	Logger::DEBUG() << "Framebuffer mapped: 0x" << (uint64_t) m_framebuffer_address << " - 0x" << (uint64_t) (m_framebuffer_address + m_framebuffer_size) << " (pages: " << pages << ")\n";

}

VideoElectronicsStandardsAssociation::~VideoElectronicsStandardsAssociation() = default;

/**
 * @brief Sets the mode of the VESA driver
 *
 * @param width Width of the screen
 * @param height Height of the screen
 * @param color_depth Color depth of the screen
 * @return True if the mode was set successfully, false otherwise
 */
bool VideoElectronicsStandardsAssociation::internal_set_mode(uint32_t width, uint32_t height, uint32_t color_depth) {

	// Can only use the mode set up already by grub
	return width == m_framebuffer_info->common.framebuffer_width
		   && height == m_framebuffer_info->common.framebuffer_height
		   && color_depth == m_framebuffer_info->common.framebuffer_bpp;

}

/**
 * @brief Checks if the VESA driver supports the given mode
 *
 * @param width The m_width of the screen
 * @param height The m_height of the screen
 * @param color_depth The color depth of the screen
 * @return
 */
bool VideoElectronicsStandardsAssociation::supports_mode(uint32_t width, uint32_t height, uint32_t color_depth) {

	// Check if the mode is supported
	return width == m_framebuffer_info->common.framebuffer_width
		   && height == m_framebuffer_info->common.framebuffer_height
		   && color_depth == m_framebuffer_info->common.framebuffer_bpp;
}

/**
 * @brief Renders a pixel on the screen in 32 bit mode
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 32bit colour of the pixel
 */
void VideoElectronicsStandardsAssociation::render_pixel_32_bit(uint32_t x, uint32_t y, uint32_t colour) {

	auto *pixel_address = (uint32_t *) ((uint8_t *) m_framebuffer_address + (m_pitch * y) + (m_bpp * x) / 8);
	*pixel_address = colour;

}

/**
 * @brief Gets the colour of a pixel on the screen in 32 bit mode
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 32bit colour of the pixel
 */
uint32_t VideoElectronicsStandardsAssociation::get_rendered_pixel_32_bit(uint32_t x, uint32_t y) {

	auto *pixel_address = (uint32_t *) ((uint8_t *) m_framebuffer_address + m_pitch * (y) + m_bpp * (x) / 8);
	return *pixel_address;
}

/**
 * @brief The name of the vendor of the VESA standard
 *
 * @return The name of the vendor
 */
string VideoElectronicsStandardsAssociation::vendor_name() {

	// Creator of the VESA standard
	return "NEC Home Electronics";
}

/**
 * @brief The name of the device
 *
 * @return The name of the device
 */
string VideoElectronicsStandardsAssociation::device_name() {
	return "VESA compatible graphics card";
}
