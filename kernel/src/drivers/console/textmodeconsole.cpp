//
// Created by 98max on 27/09/2023.
//

#include <drivers/console/textmodeconsole.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::console;

TextModeConsole::TextModeConsole()
: Driver(),
  Console(),
  videoMemory((uint16_t*) 0xb8000)
{

}

TextModeConsole::~TextModeConsole()
{

}

uint16_t TextModeConsole::getWidth()
{
    return 80;
}

uint16_t TextModeConsole::getHeight()
{
    return 25;
}

/**
 * Places a character at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The character to place
 */
void TextModeConsole::putChar(common::uint16_t x, common::uint16_t y, char c) {

    // If the coordinates are out of bounds, return
    if(x >= 80 || y >= 25)
        return;

    // Calculate the offset (*80 because there are 80 characters per line)
    int offset = (y*80 + x);

    // Set the character at the offset, by masking the character with the current character (last 8 bits)
    videoMemory[offset] = (videoMemory[offset] & 0xFF00) | (uint16_t)c;

}

/**
 * Sets the foreground color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @param foreground The foreground color
 */
void TextModeConsole::setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor foreground) {

    // If the coordinates are out of bounds, return
    if(x >= 80 || y >= 25)
        return;

    // Calculate the offset (*80 because there are 80 characters per line)
    int offset = (y*80 + x);

    // Set the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    videoMemory[offset] = (videoMemory[offset] & 0xF0FF) | ((uint16_t)foreground << 8);
}

/**
 * Sets the background color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @param background The background color
 */
void TextModeConsole::setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor background) {

    // If the coordinates are out of bounds, return
    if(x >= 80 || y >= 25)
        return;

    // Calculate the offset (*80 because there are 80 characters per line)
    int offset = (y*80 + x);

    // Set the background color at the offset, by masking the background color with the current background color (bits 12-15)
    videoMemory[offset] = (videoMemory[offset] & 0x0FFF) | ((uint16_t)background << 12);

}

/**
 * Gets the character at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The character at the specified location
 */
char TextModeConsole::getChar(common::uint16_t x, common::uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= 80 || y >= 25)
        return ' ';

    // Calculate the offset (*80 because there are 80 characters per line)
    int offset = (y*80 + x);

    // Return the character at the offset, by masking the character with the current character (last 8 bits)
    return (char)(videoMemory[offset] & 0x00FF);
}

/**
 * Gets the foreground color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The foreground color at the specified location
 */
ConsoleColor TextModeConsole::getForegroundColor(common::uint16_t x, common::uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= 80 || y >= 25)
        return ConsoleColor::Black;

    // Calculate the offset (*80 because there are 80 characters per line)
    int offset = (y*80 + x);

    // Return the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    return (ConsoleColor)((videoMemory[offset] & 0x0F00) >> 8);
}

/**
 * Gets the background color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The background color at the specified location
 */
 ConsoleColor TextModeConsole::getBackgroundColor(common::uint16_t x, common::uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= 80 || y >= 25)
        return ConsoleColor::Black;

    // Calculate the offset (*80 because there are 80 characters per line)
    int offset = (y*80 + x);

    // Return the background color at the offset, by masking the background color with the current background color (bits 12-15)
    return (ConsoleColor)((videoMemory[offset] & 0xF000) >> 12);
}



