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

VideoElectronicsStandardsAssociation::VideoElectronicsStandardsAssociation(multiboot_tag_framebuffer* framebuffer_info)
: VideoDriver(),
  m_framebuffer_info(framebuffer_info)
{
  // Get the framebuffer info
  Logger::INFO() << "Setting up VESA driver\n";
  Logger::DEBUG() << "Framebuffer info: 0x " << (uint64_t)m_framebuffer_info << "\n";

  // Set the framebuffer address, bpp and pitch
  m_bpp = m_framebuffer_info->common.framebuffer_bpp;
  m_pitch = m_framebuffer_info->common.framebuffer_pitch;
  m_framebuffer_size = m_framebuffer_info->common.framebuffer_height * m_pitch;

  Logger::DEBUG() << "Framebuffer: bpp=" << m_bpp << ", pitch=" << m_pitch << ", size=" << m_framebuffer_size << "\n";

  // Get the framebuffer address
  auto physical_address = (uint64_t)m_framebuffer_info->common.framebuffer_addr;
  auto virtual_address = (uint64_t)PhysicalMemoryManager::to_dm_region(physical_address);
  uint64_t end = physical_address + m_framebuffer_size;
  m_framebuffer_address = (uint64_t*)virtual_address;

  Logger::DEBUG() << "Framebuffer address: physical=0x" << physical_address << ", virtual=0x" << virtual_address << "\n";

  // Map the framebuffer
  while (physical_address < end) {

    PhysicalMemoryManager::s_current_manager->map((physical_address_t*)physical_address, (virtual_address_t*)virtual_address, Write | Present);
    physical_address += PhysicalMemoryManager::s_page_size;
    virtual_address += PhysicalMemoryManager::s_page_size;
  }

  size_t pages = PhysicalMemoryManager::size_to_frames(virtual_address - (uint64_t)m_framebuffer_address);
  Logger::DEBUG() << "Framebuffer mapped: 0x" << (uint64_t)m_framebuffer_address << " - 0x" << virtual_address << " (pages: " << pages << ")\n";

  // Reserve the physical memory
  PhysicalMemoryManager::s_current_manager->reserve(m_framebuffer_info->common.framebuffer_addr, pages);

  // Set the default video mode
  this -> set_mode(framebuffer_info->common.framebuffer_width,framebuffer_info->common.framebuffer_height, framebuffer_info->common.framebuffer_bpp);

}

VideoElectronicsStandardsAssociation::~VideoElectronicsStandardsAssociation()= default;

/**
 * @brief Initializes the VESA driver
 *
 * @return True if the driver was initialized successfully, false otherwise
 */
bool VideoElectronicsStandardsAssociation::init() {

    //Multiboot inits this for us
    return true;
}

/**
 * @brief Sets the mode of the VESA driver
 *
 * @param width Width of the screen
 * @param height Height of the screen
 * @param color_depth Color depth of the screen
 * @return True if the mode was set successfully, false otherwise
 */
bool VideoElectronicsStandardsAssociation::internal_set_mode(uint32_t, uint32_t, uint32_t) {

    // Best mode is set by the bootloader
    return true;


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
    if(width == (uint32_t)m_framebuffer_info->common.framebuffer_width && height == (uint32_t)m_framebuffer_info->common.framebuffer_height && color_depth == (uint32_t)m_framebuffer_info->common.framebuffer_bpp) {
        return true;
    }
    return false;
}

/**
 * @brief Renders a pixel on the screen in 32 bit mode
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 32bit colour of the pixel
 */
void VideoElectronicsStandardsAssociation::render_pixel_32_bit(uint32_t x, uint32_t y, uint32_t colour) {

    // Get the address of the pixel
    auto* pixel_address = (uint32_t*)((uint8_t *)m_framebuffer_address + m_pitch * (y) + m_bpp * (x) / 8);

    // Set the pixel
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

    // Get the address of the pixel
    auto* pixel_address = (uint32_t*)((uint8_t *)m_framebuffer_address + m_pitch * (y) + m_bpp * (x) / 8);

    // Return the pixel
    return *pixel_address;
}

/**
 * @brief The name of the vendor of the VESA standard
 *
 * @return The name of the vendor
 */
string VideoElectronicsStandardsAssociation::vendor_name() {
    return "NEC Home Electronics";  // Creator of the VESA standard
}

/**
 * @brief The name of the device
 *
 * @return The name of the device
 */
string VideoElectronicsStandardsAssociation::device_name() {
    return "VESA compatible graphics card";
}
