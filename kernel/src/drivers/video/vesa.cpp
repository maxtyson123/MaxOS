//
// Created by 98max on 19/07/2023.
//

#include <drivers/video/vesa.h>

using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::video;
using namespace maxOS::memory;
using namespace maxOS::system;

VideoElectronicsStandardsAssociationDriver::VideoElectronicsStandardsAssociationDriver(MemoryManager* memoryManager,  multiboot_info_t* mb_info)
        : VideoDriver()
{
    // Store the memory manager
    this->memoryManager = memoryManager;

    // Store the multiboot info
    this->multibootInfo = mb_info;

    // Info from multiboot
    this->framebufferAddress = (uint32_t*)multibootInfo->framebuffer_addr;
    this->bpp = multibootInfo->framebuffer_bpp;
    this->pitch = multibootInfo->framebuffer_pitch;


}

VideoElectronicsStandardsAssociationDriver::~VideoElectronicsStandardsAssociationDriver(){

}

bool VideoElectronicsStandardsAssociationDriver::init() {

    //Multiboot inits this for us
    return true;
}

/**
 * @brief Sets the mode of the VESA driver (TODO: List of modes)
 * @param width Width of the screen
 * @param height Height of the screen
 * @param colorDepth Color depth of the screen
 * @return True if the mode was set successfully, false otherwise
 */
bool VideoElectronicsStandardsAssociationDriver::internalSetMode(uint32_t width, uint32_t height, uint32_t colorDepth) {


    // Check if the mode is supported
    if(!supportsMode(width, height, colorDepth)) {
        // Mode is not supported
        return false;
    }

    // Initialise the VESA driver
    if(!init()) {
        // VESA driver could not be initialised
        return false;
    }

    // Return true if the mode was set successfully
    return true;


}

/**
 * @brief Checks if the VESA driver supports the given mode
 * @param width The width of the screen
 * @param height The height of the screen
 * @param colorDepth The color depth of the screen
 * @return
 */
bool VideoElectronicsStandardsAssociationDriver::supportsMode(uint32_t width, uint32_t height, uint32_t colorDepth) {

    // Will change to detect mode later
    if(width == 640 && height == 480 && colorDepth == 16) return true;
    if(width == 800 && height == 600 && colorDepth == 16) return true;
    if(width == 1024 && height == 768 && colorDepth == 32) return true;
    return false;
}

/**
 * @brief Renders a pixel on the screen in 32 bit mode
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 32bit colour of the pixel
 */
void VideoElectronicsStandardsAssociationDriver::renderPixel32Bit(uint32_t x, uint32_t y, uint32_t colour) {

    // Get the address of the pixel
    uint32_t* pixelAddress = (uint32_t*)((uint8_t *)framebufferAddress + pitch * (y) + bpp * (x) / 8);

    // Set the pixel
    *pixelAddress = colour;

}

uint32_t VideoElectronicsStandardsAssociationDriver::getRenderedPixel32Bit(uint32_t x, uint32_t y) {
    // Get the address of the pixel
    uint32_t* pixelAddress = (uint32_t*)((uint8_t *)framebufferAddress + pitch * (y) + bpp * (x) / 8);

    // Set the pixel
    return *pixelAddress;                                           // Return the colour of the pixel
}