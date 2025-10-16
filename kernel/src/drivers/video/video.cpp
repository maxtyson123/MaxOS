//
// Created by 98max on 8/04/2023.
//

#include <drivers/video/video.h>

using namespace MaxOS::drivers::video;
using namespace MaxOS::common;

VideoDriver::VideoDriver() = default;

VideoDriver::~VideoDriver() = default;

/**
 * @brief Set the mode of the video driver
 *
 * @param width The width of the screen
 * @param height The height of the screen
 * @param color_depth The color depth of the screen
 * @return true If the mode was set successfully false otherwise
 */
bool VideoDriver::internal_set_mode(uint32_t, uint32_t, uint32_t) {
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
bool VideoDriver::supports_mode(uint32_t, uint32_t, uint32_t) {
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
bool VideoDriver::set_mode(uint32_t width, uint32_t height, uint32_t color_depth) {

    // Cant set it if not supported
    if(!supports_mode(width, height, color_depth))
        return false;

    // Try set the mode
    if(!internal_set_mode(width, height, color_depth))
        return false;

    // Store the mode
    m_width = width;
    m_height = height;
    m_color_depth = color_depth;
    return true;
}