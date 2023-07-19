//
// Created by 98max on 19/07/2023.
//

#include <drivers/video/vesa.h>

using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::video;

VideoElectronicsStandardsAssociationDriver::VideoElectronicsStandardsAssociationDriver(memory::MemoryManager* memoryManager)
: VideoDriver()
{
    // Store the memory manager
    this->memoryManager = memoryManager;

    // Frame buffer is set when the mode is set
    this->framebufferAddress = 0;

}

VideoElectronicsStandardsAssociationDriver::~VideoElectronicsStandardsAssociationDriver(){

}

/**
 * @brief Sets the mode of the VESA driver (TODO: List of modes)
 * @param width Width of the screen
 * @param height Height of the screen
 * @param colorDepth Color depth of the screen
 * @return True if the mode was set successfully, false otherwise
 */
bool VideoElectronicsStandardsAssociationDriver::internalSetMode(uint32_t width, uint32_t height, uint32_t colorDepth) {

    // Allocate memory for the frame buffer
    uint32_t size = width * height * (colorDepth / 8);
    this->framebufferAddress = (uint8_t*) this->memoryManager->malloc(size);

    // Mode setting code..

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

    // If there is no frame buffer, return
    if(this->framebufferAddress == 0) return;

    // Get the address of the pixel
    uint32_t* pixelAddress = (uint32_t*)(framebufferAddress + y*width*(colorDepth/8) + x*(colorDepth/8));

    // Set the pixel
    *pixelAddress = colour;

}
