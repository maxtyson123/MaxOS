//
// Created by 98max on 19/07/2023.
//

#include <drivers/video/vesa.h>

using namespace maxOS;
using namespace maxOS::drivers;
using namespace maxOS::drivers::video;
using namespace maxOS::memory;
using namespace maxOS::system;

VideoElectronicsStandardsAssociation::VideoElectronicsStandardsAssociation(multiboot_tag_framebuffer* framebuffer_info)
: VideoDriver(),
  m_framebuffer_info(framebuffer_info),
  m_framebuffer_address((uint64_t *)framebuffer_info->common.framebuffer_addr),
  m_bpp(framebuffer_info->common.framebuffer_bpp),
  m_pitch(framebuffer_info->common.framebuffer_pitch)
{
}

VideoElectronicsStandardsAssociation::~VideoElectronicsStandardsAssociation(){

}

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
bool VideoElectronicsStandardsAssociation::internal_set_mode(uint32_t width, uint32_t height, uint32_t color_depth) {

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
    uint32_t*pixel_address = (uint32_t*)((uint8_t *)m_framebuffer_address + m_pitch * (y) + m_bpp * (x) / 8);

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
    uint32_t*pixel_address = (uint32_t*)((uint8_t *)m_framebuffer_address + m_pitch * (y) + m_bpp * (x) / 8);

    // Return the pixel
    return *pixel_address;
}

/**
 * @brief Renders a pixel on the screen in 16 bit mode
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 16bit colour of the pixel
 */
string VideoElectronicsStandardsAssociation::get_vendor_name() {
    return "NEC Home Electronics";  // Creator of the VESA standard
}

/**
 * @brief Gets the colour of a pixel on the screen in 16 bit mode
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 16bit colour of the pixel
 */
string VideoElectronicsStandardsAssociation::get_device_name() {
    return "VESA compatible graphics card";
}
