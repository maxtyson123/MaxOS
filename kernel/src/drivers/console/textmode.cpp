//
// Created by 98max on 27/09/2023.
//

#include <drivers/console/textmode.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::console;

TextModeConsole::TextModeConsole()
: Driver(),
  Console()
{

}

TextModeConsole::~TextModeConsole()
{

}

/**
 * @brief Gets the width of the console
 *
 * @return The width of the console in characters
 */
uint16_t TextModeConsole::width()
{
    return 80;
}

/**
 * @brief Gets the height of the console
 *
 * @return The height of the console in characters
 */
uint16_t TextModeConsole::height()
{
    return 25;
}

/**
 * @brief Places a character at the specified location if it is in bounds
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The character to place
 */
void TextModeConsole::put_character(uint16_t x, uint16_t y, char c) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return;

    // Calculate the offset 
    int offset = (y*width() + x);

    // Set the character at the offset, by masking the character with the current character (last 8 bits)
    m_video_memory[offset] = (m_video_memory[offset] & 0xFF00) | (uint16_t)c;

}

/**
 * @brief Sets the foreground color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param foreground The foreground color
 */
void TextModeConsole::set_foreground_color(uint16_t x, uint16_t y, ConsoleColour foreground) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return;

    // Calculate the offset 
    int offset = (y* width() + x);

    // Set the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    m_video_memory[offset] = (m_video_memory[offset] & 0xF0FF) | ((uint16_t)foreground << 8);
}

/**
 * @brief Sets the background color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param background The background color
 */
void TextModeConsole::set_background_color(uint16_t x, uint16_t y, ConsoleColour background) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return;

    // Calculate the offset 
    int offset = (y* width() + x);

    // Set the background color at the offset, by masking the background color with the current background color (bits 12-15)
    m_video_memory[offset] = (m_video_memory[offset] & 0x0FFF) | ((uint16_t)background << 12);

}

/**
 * @brief Gets the character at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The character at the specified location or a space if the coordinates are out of bounds
 */
char TextModeConsole::get_character(uint16_t x, uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return ' ';

    // Calculate the offset 
    int offset = (y* width() + x);

    // Return the character at the offset, by masking the character with the current character (last 8 bits)
    return (char)(m_video_memory[offset] & 0x00FF);
}

/**
 * @brief Gets the foreground color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The foreground color at the specified location or white if the coordinates are out of bounds
 */
ConsoleColour TextModeConsole::get_foreground_color(uint16_t x, uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return ConsoleColour::White;

    // Calculate the offset 
    int offset = (y* width() + x);

    // Return the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    return (ConsoleColour)((m_video_memory[offset] & 0x0F00) >> 8);
}

/**
 * @brief Gets the background color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The background color at the specified location or black if the coordinates are out of bounds
 */
ConsoleColour TextModeConsole::get_background_color(uint16_t x, uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return ConsoleColour::Black;

    // Calculate the offset 
    int offset = (y* width() + x);

    // Return the background color at the offset, by masking the background color with the current background color (bits 12-15)
    return (ConsoleColour)((m_video_memory[offset] & 0xF000) >> 12);
}