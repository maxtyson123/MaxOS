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

/**
 * @brief Set the mode of the video driver
 *
 * @param width The width of the screen
 * @param height The height of the screen
 * @param color_depth The color depth of the screen
 * @return true If the mode was set successfully false otherwise
 */
bool VideoDriver::internal_set_mode(uint32_t width, uint32_t height, uint32_t color_depth) {
    return false;
}

/**
 * @brief Check if the video driver supports a certain mode
 *
 * @param width The width of the screen
 * @param height The height of the screen
 * @param color_depth The color depth of the screen
 * @return true If the mode is supported, false otherwise
 */
bool VideoDriver::supports_mode(uint32_t width, uint32_t height, uint32_t color_depth) {
    return false;
}

/**
 * @brief Set the mode of the video driver
 *
 * @param width The width of the screen
 * @param height The height of the screen
 * @param color_depth The color depth of the screen
 * @return true If the mode was set successfully (and the screen was updated) false otherwise
 */
bool VideoDriver::set_mode(uint32_t width, uint32_t height, uint32_t colorDepth) {

    // Check if the mode is supported
    if(!supports_mode(width, height, colorDepth))
        return false;

    // Set the mode
    if(internal_set_mode(width, height, colorDepth))
    {
        this -> m_width = width;
        this -> m_height = height;
        this -> m_color_depth = colorDepth;
        return true;
    }

    // If setting the mode failed, return false
    return false;
}