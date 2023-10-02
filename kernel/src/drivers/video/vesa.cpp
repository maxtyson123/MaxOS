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

    // Frame buffer is set when the mode is set
    this->framebufferAddress = 0;

}

VideoElectronicsStandardsAssociationDriver::~VideoElectronicsStandardsAssociationDriver(){

}

bool VideoElectronicsStandardsAssociationDriver::init() {

    // Check if VESA is supported
    if(!(this->multibootInfo->flags & MULTIBOOT_INFO_VBE_INFO)) {
        // VESA is not supported
        return false;
    }

    // Get the VESA control info and mode info from the multiboot info
    vesa_control_info_t* multibootVESAControlInfo = (vesa_control_info_t*) this->multibootInfo->vbe_control_info;
    vesa_mode_info_t* multibootVESAModeInfo = (vesa_mode_info_t*) this->multibootInfo->vbe_mode_info;

    // Copy the VESA control info and mode info to the VESA driver
    memcpy(&this->vesaControlInfo, multibootVESAControlInfo, sizeof(vesa_control_info_t));
    memcpy(&this->vesaModeInfo, multibootVESAModeInfo, sizeof(vesa_mode_info_t));

    /*
    // Check if VESA is available
    bool vesa_available =
            (multibootVESAModeInfo->attributes & 0x80) &&       // The frame buffer is available
            (multibootVESAModeInfo->attributes & 0x10) &&       // The mode is supported by the hardware
             multibootVESAModeInfo->memory_model == 0x6;        // Make sure it is in direct colour mode not packed pixel
    if(!vesa_available) {
        // VESA is not available
        return false;
    }
     */

    // Get the framebuffer address
    this->framebufferAddress = (uint32_t*) this->vesaModeInfo->framebuffer;     // GDB -> 0x10080300

    //color code macros:
#define red   0xFF0000
#define green 0x00FF00
#define blue  0x0000FF

    /*store the framebuffer address inside a new pointer
      the framebuffer is located at MultiBootHeaderStruct[22]*/
    unsigned int* framebuffer = (uint32_t*) this->vesaModeInfo->framebuffer;

    /*now to place a pixel at a specific screen position(screen index starts at
      the top left of the screen which is at index 0), write a color value
      to the framebuffer pointer with a specified index*/

    //Sample Code:
    framebuffer[0] = red; //writes a red pixel(0xFF0000) at screen index 0
    framebuffer[1] = green; //writes a green pixel(0x00FF00) at screen index 1
    framebuffer[2] = blue; //writes a blue pixel(0x0000FF) at screen index 2
    /*this code should plot 3 pixels at the very top left of the screen
      one red, one green and one blue. If you want a specific color you can
      use the link I will provide at the bottom.*/


    //Sample code to fil the entire screen blue:
    int totalPixels = 480000; //800x600 = 480000 pixels total

    //loop through each pixel and write a blue color value to it
    for (int i = 0; i < totalPixels; i++)
    {
        framebuffer[i] = blue; //0x0000FF is the blue color value
    }

    // VESA is supported and set up
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

    // If there is no frame buffer, return
    if(framebufferAddress == 0) return;

    // Get the address of the pixel
    uint32_t* pixelAddress = (uint32_t*)((uint8_t *)framebufferAddress + vesaModeInfo -> pitch * (y) + vesaModeInfo->bpp * (x) / 8);

    // Set the pixel
    *pixelAddress = colour;  // TODO: GDB shows that colour is not in lil endian format

}


//TODO: Fix Rendering Pixels