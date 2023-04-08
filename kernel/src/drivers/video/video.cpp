//
// Created by 98max on 8/04/2023.
//

#include <drivers/video/video.h>

using namespace maxOS::drivers::video;
using namespace maxOS::common;

VideoDriver::VideoDriver()
: Driver(),
  GraphicsContext()
{

}

VideoDriver::~VideoDriver() {

}

bool VideoDriver::internalSetMode(uint32_t width, uint32_t height, uint32_t colorDepth) {
    return false;
}

bool VideoDriver::supportsMode(uint32_t width, uint32_t height, uint32_t colorDepth) {
    return false;
}

bool VideoDriver::setMode(uint32_t width, uint32_t height, uint32_t colorDepth) {

    // Check if the mode is supported
    if(!supportsMode(width, height, colorDepth))
        return false;

    // Set the mode
    if(internalSetMode(width, height, colorDepth))
    {
        this -> width = width;
        this -> height = height;
        this -> colorDepth = colorDepth;
        return true;
    }

    // If setting the mode failed, return false
    return false;
}

bool VideoDriver::setTextMode() {
    return false;
}

